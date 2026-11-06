#include "scoundrel.h"

#include "SDL3/SDL_scancode.h"
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
    // TODO: Iniciar el juego mueve 4 cartas y las da vuelta al room

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

    Dungeon[Dungeon.size() - 1].Position = Room1;
    Dungeon[Dungeon.size() - 2].Position = Room2;
    Dungeon[Dungeon.size() - 3].Position = Room3;
    Dungeon[Dungeon.size() - 4].Position = Room4;
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
        if(ClickedCard && ClickedCard->Pile == Pile_Dungeon)
        {
            if(!GameStarted)
            {
                MoveCards(Dungeon, Room, 4);
                GameStarted = true;
            }
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

            // Stack the cards, each one slightly above the previous one so the top of the pile is the closest to the camera
            glm::vec3 Position = DungeonPosition + glm::vec3(0.0f, 0.0f, CardThickness * (f32)Dungeon.size());

            // Face down, the back faces the camera
            card Card = CreateCard(FrontTexture, CardBackTexture, Position, glm::pi<f32>(), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f), Pile_Dungeon);
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
}

void game::DrawCards()
{
    for(const auto &Card : Dungeon)
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

void game::MoveCards(std::vector<card> &Src, std::vector<card> &Dst, u32 Count)
{
    // Moves cards from one pile to the other
    Log(Info, "Moving Cards!\n");
}
