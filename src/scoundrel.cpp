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
    PlayerHealth = 20;
    DungeonPosition = glm::vec3(-2.5f, -1.15f, 0.0f);

    Camera = CreateCamera();
    Camera.Position = glm::vec3(0.0f, 0.0f, 2.0f);
    LoadDungeon();
}

void game::Update()
{
    Camera.Update();

    // TODO: Make this work!, Move PickCard and ScreenToWorldRay somewhere else
    #if 0
    b32 MouseDown    = Mouse->IsButtonPressed(SDL_BUTTON_LEFT);
    b32 MouseWasDown = Mouse->WasButtonPressed(SDL_BUTTON_LEFT);
    if(MouseDown && !MouseWasDown)
    {
        glm::vec3 RayDirection = ScreenToWorldRay(Mouse, Renderer, &Camera);
        card *ClickedCard = PickCard(Dungeon, Camera.Position, RayDirection);
        if(ClickedCard && ClickedCard->Pile == Pile_Dungeon)
        {
            Log(Info, "Clicked the dungeon");
        }
    }
    #endif
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
