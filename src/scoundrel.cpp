#include "scoundrel.h"

#include "asset_manager.h"
#include "camera.h"
#include "typedefs.h"

#include <algorithm>
#include <random>
#include <vector>

void game::Init(renderer *RendererPtr, audio_system *AudioPtr, keyboard *KeyboardPtr, mouse *MousePtr, asset_manager *AssetMgrPtr)
{
    Assert(RendererPtr && AudioPtr && KeyboardPtr && MousePtr && AssetMgrPtr);

    Renderer = RendererPtr;
    Mouse = MousePtr;
    Keyboard = KeyboardPtr;
    Audio = AudioPtr;
    AssetMgr = AssetMgrPtr;

    // Load Assets
    MarcellusFont = AssetMgr->LoadBitmapFont("assets/Fonts/Marcellus_sc.png", "assets/Fonts/Marcellus_sc.json");
    CardBackTexture = AssetMgr->LoadTexture("assets/Textures/Scoundrel-Back.jpg");

    // Setup game vars
    GameStarted = false;
    PlayerHealth = 20;

    DungeonPosition = glm::vec3(-22.1f, -10.0f, 0.0f);
    DiscardPosition = glm::vec3(22.1f, -10.0f, 0.0f);
    WeaponPosition = glm::vec3(12.1f, -10.0f, 0.0f);

    // TODO: Dibujar los textos arriba de cada deck
    // TODO: Dibujar todos los deck en Render()

    Room1 = glm::vec3(-14.0, 10.0f, 0.0f);
    Room2 = glm::vec3(-14.0 + 8.5f, 10.0f, 0.0f);
    Room3 = glm::vec3(-14.0 + 8.5f * 2, 10.0f, 0.0f);
    Room4 = glm::vec3(-14.0 + 8.5f * 3, 10.0f, 0.0f);

    // DungeonTextPosition = ;
    // RoomTextPosition = ;
    // DiscardTextPosition = ;

    // Camera
    glm::vec3 CameraPosition = glm::vec3(0.0f, 0.0f, 45.0f);
    glm::vec3 CameraTarget = glm::vec3(0.0f);
    glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    f32 CameraNear = 1.0f;
    f32 CameraFar = 1000.0f;
    f32 CameraFoV = 40.0f;
    Camera = CreateCamera(CameraPosition, CameraTarget, CameraUp, CameraNear, CameraFar, CameraFoV);

    LoadDungeon();
}

card *game::PickCard(glm::vec3 RayOrigin, glm::vec3 RayDirection)
{
    card *Closest = nullptr;
    f32 ClosestDistance = FLT_MAX;

    for(card &Card : Dungeon)
    {
        // The plane the card lies on, it tilts with the card
        glm::vec3 CardNormal = Card.RotationMatrix() * glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 Hit;
        if(RayPlaneIntersection(RayOrigin, RayDirection, Card.Position, CardNormal, &Hit) && Card.ContainsPoint(Hit))
        {
            // NOTE: Strict < keeps the first card on ties, cards at the same Z are drawn first on top (depth test is GL_LESS)
            f32 Distance = glm::length(Hit - RayOrigin);
            if(Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                Closest = &Card;
            }
        }
    }

    return Closest;
}

void game::Update()
{
    Camera.Update();

    b32 MouseDown    = Mouse->IsButtonPressed(SDL_BUTTON_LEFT);
    b32 MouseWasDown = Mouse->WasButtonPressed(SDL_BUTTON_LEFT);
    if(MouseDown && !MouseWasDown)
    {
        glm::vec3 RayDirection = ScreenToWorldRay(Mouse, Renderer, &Camera);
        card *ClickedCard = PickCard(Camera.Position, RayDirection);
        // NOTE: ClickedCard points into Dungeon, don't use it after DealRoom() changes Dungeon
        if(ClickedCard && ClickedCard->Pile == Pile_Dungeon && !GameStarted)
        {
            DealRoom();
            GameStarted = true;
        }
    }
}

void game::Render()
{
    Renderer->BeginFrame();

    DrawHealth();
    DrawCards();

    Renderer->EndFrame();
}

void game::LoadDungeon()
{
    const char *SuitNames[Suit_Count] = {"Clubs", "Spades", "Diamonds", "Hearts"};
    const char *RankNames[] = {"", "", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"}; // Indexed by Value

    Dungeon.reserve(44);

    for(i32 Suit = 0; Suit < Suit_Count; ++Suit)
    {
        b32 IsRed = (Suit == Suit_Diamonds || Suit == Suit_Hearts);
        i32 MaxValue = IsRed ? 10 : 14;

        for(i32 Value = 2; Value <= MaxValue; ++Value)
        {
            char Path[128];
            snprintf(Path, sizeof(Path), "assets/Textures/Scoundrel-%s-%s.jpg", SuitNames[Suit], RankNames[Value]);

            u64 FrontTexture = AssetMgr->LoadTexture(Path);

            // Face down, the back faces the camera. The pile height is set after shuffling
            card Card = CreateCard(FrontTexture, CardBackTexture, DungeonPosition, glm::pi<f32>(), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f), Pile_Dungeon);
            Card.Suit = (card_suit)Suit;
            Card.Type = (Suit == Suit_Diamonds) ? Card_Weapon : (Suit == Suit_Hearts) ? Card_Potion : Card_Monster;
            Card.Value = Value;

            Dungeon.push_back(Card);
        }
    }

    Log(Info, "LoadDungeon() - Loaded %zu cards", Dungeon.size());
    Assert(Dungeon.size() == 44);

    // Shuffle the cards
    std::random_device RandomDevice;
    std::mt19937 Generator(RandomDevice());
    std::shuffle(Dungeon.begin(), Dungeon.end(), Generator);

    // Stack the shuffled cards, the last card is the top of the pile (closest to the camera)
    for(size_t i = 0; i < Dungeon.size(); ++i)
    {
        Dungeon[i].Position = DungeonPosition + glm::vec3(0.0f, 0.0f, CardThickness * (f32)i);
    }
}

void game::DealRoom()
{
    glm::vec3 RoomPositions[4] = {Room1, Room2, Room3, Room4};

    u32 Missing = 4 - (u32)Room.size();
    MoveCards(Dungeon, Room, Missing, Pile_Room);

    for(size_t i = 0; i < Room.size(); ++i)
    {
        Room[i].Position = RoomPositions[i];
        Room[i].Angle = 0.0f; // Face up
    }
}

void game::DrawCards()
{
    for(const auto &Card : Dungeon)
    {
        Renderer->DrawCard(Card);
    }

    for(const auto &Card : Room)
    {
        Renderer->DrawCard(Card);
    }
}

void game::DrawHealth()
{
    glm::vec2 ScreenPos = glm::vec2(0.0f, 0.0f);
    f32 Size = 40.0f;
    char TextBuffer[128];
    snprintf(TextBuffer, sizeof(TextBuffer), "Health: %d", PlayerHealth);
    Renderer->DrawText(MarcellusFont, ScreenPos.x, ScreenPos.y, Size, TextBuffer, RED);
}

// Moves the top Count cards of Src (the back of the vector) to the back of Dst, one by one like dealing
void game::MoveCards(std::vector<card> &Src, std::vector<card> &Dst, u32 Count, card_pile DstPile)
{
    for(u32 i = 0; i < Count && !Src.empty(); ++i)
    {
        card Card = Src.back();
        Src.pop_back();
        Card.Pile = DstPile;
        Dst.push_back(Card);
    }
}
