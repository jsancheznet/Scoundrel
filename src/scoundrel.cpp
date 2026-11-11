#include "scoundrel.h"

#include "asset_manager.h"
#include "bitmap_font.h"
#include "camera.h"
#include "typedefs.h"

#include <SDL3/SDL.h>

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

    SlashSound        = AssetMgr->LoadSound("assets/Sounds/Slash.wav", Channel_SFX);
    PunchSound        = AssetMgr->LoadSound("assets/Sounds/Punch.wav", Channel_SFX);
    DamageSound       = AssetMgr->LoadSound("assets/Sounds/Damage.wav", Channel_SFX);
    HealthPotionSound = AssetMgr->LoadSound("assets/Sounds/HealthPotion.wav", Channel_SFX);
    ErrorSound        = AssetMgr->LoadSound("assets/Sounds/Error.wav", Channel_SFX);
    Music             = AssetMgr->LoadSound("assets/Sounds/Music.wav", Channel_Music);

    // The music starts once here and repeats for the whole session, NewGame() doesn't restart it
    Audio->SetChannelVolume(Channel_SFX, SfxVolume);
    Audio->SetChannelVolume(Channel_Music, MusicVolume);
    Audio->SetRepeat(Music, true);
    Audio->Play(Music);

    // Table layout from the rules: Dungeon on the left, Room in a row, Discard on the right, Weapon below the Room
    DungeonPosition = glm::vec3(-22.1f, -10.0f, 0.0f);
    DiscardPosition = glm::vec3(22.1f, -10.0f, 0.0f);
    WeaponPosition = glm::vec3(-8.0f, -10.0f, 0.0f); // Left of the center, the slain monsters stack to the right of it

    f32 RoomY = 8.5f;
    f32 RoomSpacing = 8.5f;
    f32 RoomX = -RoomSpacing * 1.5f; // Centers the row of 4 cards on X = 0
    RoomCardPosition1 = glm::vec3(RoomX, RoomY, 0.0f);
    RoomCardPosition2 = glm::vec3(RoomX + RoomSpacing, RoomY, 0.0f);
    RoomCardPosition3 = glm::vec3(RoomX + RoomSpacing * 2, RoomY, 0.0f);
    RoomCardPosition4 = glm::vec3(RoomX + RoomSpacing * 3, RoomY, 0.0f);

    // Camera
    glm::vec3 CameraPosition = glm::vec3(0.0f, 0.0f, 45.0f);
    glm::vec3 CameraTarget = glm::vec3(0.0f);
    glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    f32 CameraNear = 1.0f;
    f32 CameraFar = 1000.0f;
    f32 CameraFoV = 40.0f;
    Camera = CreateCamera(CameraPosition, CameraTarget, CameraUp, CameraNear, CameraFar, CameraFoV);

    LoadDungeon();
    NewGame();
}

// Returns the clickable card closest to the camera under the ray (Dungeon or Room), nullptr if nothing was hit
// NOTE: The pointer is only valid until the piles change, use it before moving any card
card *game::PickCard(glm::vec3 RayOrigin, glm::vec3 RayDirection)
{
    card *Closest = nullptr;
    f32 ClosestDistance = FLT_MAX;

    std::vector<card> *ClickablePiles[] = {&Dungeon, &Room};
    for(std::vector<card> *Pile : ClickablePiles)
    {
        for(card &Card : *Pile)
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
    }

    return Closest;
}

void game::Update()
{
    Camera.Update();
    ProcessInput();

    // After the input, so the hint always matches the state this frame is drawn with
    UpdateActionHint();
}

// Handles the R key and the clicks on the Dungeon and the Room
void game::ProcessInput()
{
    // R starts a new game once the current one is over
    if((State == Game_Won || State == Game_Lost) && Keyboard->IsReleased(SDL_SCANCODE_R))
    {
        NewGame();
        return;
    }

    // Left click: use the weapon on monsters when possible. Right click: fight monsters barehanded
    b32 LeftClicked  = Mouse->IsButtonPressed(SDL_BUTTON_LEFT) && !Mouse->WasButtonPressed(SDL_BUTTON_LEFT);
    b32 RightClicked = Mouse->IsButtonPressed(SDL_BUTTON_RIGHT) && !Mouse->WasButtonPressed(SDL_BUTTON_RIGHT);
    if(!LeftClicked && !RightClicked)
    {
        return;
    }

    glm::vec3 RayDirection = ScreenToWorldRay(Mouse, Renderer, &Camera);
    card *ClickedCard = PickCard(Camera.Position, RayDirection);
    if(!ClickedCard)
    {
        return;
    }

    // NOTE: ClickedCard points into a pile, don't use it after the piles change
    if(ClickedCard->Pile == Pile_Dungeon)
    {
        if(State == Game_WaitingToStart)
        {
            DealRoom();
            State = Game_Playing;
        }
        else if(State == Game_Playing)
        {
            // Clicking the Dungeon during a turn runs away from the Room
            AvoidRoom();
        }
    }
    else if(ClickedCard->Pile == Pile_Room && State == Game_Playing)
    {
        u32 Index = (u32)(ClickedCard - &Room[0]);
        FaceRoomCard(Index, RightClicked);
    }
}

void game::Render()
{
    Renderer->BeginFrame();

    DrawHealth();
    DrawActionHint();
    DrawCards();
    DrawLabels();

    Renderer->EndFrame();
}

void game::LoadDungeon()
{
    const char *SuitNames[Suit_Count] = {"Clubs", "Spades", "Diamonds", "Hearts"};
    const char *RankNames[] = {"", "", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"}; // Indexed by Value

    Dungeon.reserve(44);

    for(i32 Suit = 0; Suit < Suit_Count; ++Suit)
    {
        // Jokers, Red Face Cards and Red Aces are not used, so the red suits only go up to 10
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
}

// Gathers every card back into the Dungeon, shuffles it and resets the player. The textures are only loaded once
void game::NewGame()
{
    std::vector<card> *Piles[] = {&Room, &Weapon, &SlainMonsters, &DiscardPile};
    for(std::vector<card> *Pile : Piles)
    {
        MoveCards(*Pile, Dungeon, (u32)Pile->size(), Pile_Dungeon);
    }

    // Shuffle the cards
    std::random_device RandomDevice;
    std::mt19937 Generator(RandomDevice());
    std::shuffle(Dungeon.begin(), Dungeon.end(), Generator);
    StackDungeon();

    State = Game_WaitingToStart;
    PlayerHealth = MaxHealth;
    Score = 0;

    CardsFacedThisRoom = 0;
    PotionUsedThisRoom = false;
    AvoidedLastRoom = false;
    LastCardWasPotion = false;
    LastPotionValue = 0;
}

// Places the Dungeon cards face down, stacked so the last card is the top of the pile (closest to the camera)
void game::StackDungeon()
{
    for(size_t i = 0; i < Dungeon.size(); ++i)
    {
        Dungeon[i].Pile = Pile_Dungeon;
        Dungeon[i].Position = DungeonPosition + glm::vec3(0.0f, 0.0f, CardThickness * (f32)i);
        Dungeon[i].Angle = glm::pi<f32>();
        Dungeon[i].Axis = glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

// Flips cards off the top of the Dungeon until the Room has 4 face up cards, this starts a new turn
void game::DealRoom()
{
    glm::vec3 RoomPositions[RoomSize] = {RoomCardPosition1, RoomCardPosition2, RoomCardPosition3, RoomCardPosition4};

    u32 Missing = RoomSize - (u32)Room.size();
    MoveCards(Dungeon, Room, Missing, Pile_Room);

    for(size_t i = 0; i < Room.size(); ++i)
    {
        Room[i].Position = RoomPositions[i];
        Room[i].Angle = 0.0f; // Face up
    }

    CardsFacedThisRoom = 0;
    PotionUsedThisRoom = false;
}

// Scoops the whole Room to the bottom of the Dungeon and deals a new one.
// Only allowed before facing any card of the Room, and never for two Rooms in a row
void game::AvoidRoom()
{
    // NOTE: With an empty Dungeon, avoiding would just deal the same Room again
    if(CardsFacedThisRoom > 0 || AvoidedLastRoom || Room.size() != RoomSize || Dungeon.empty())
    {
        Log(Info, "game::AvoidRoom() - This Room can't be avoided");
        Audio->Play(ErrorSound);
        return;
    }

    // The bottom of the Dungeon is the front of the vector
    Dungeon.insert(Dungeon.begin(), Room.begin(), Room.end());
    Room.clear();
    StackDungeon();

    DealRoom();
    AvoidedLastRoom = true;
}

// Faces the Room card at Index: weapons are equipped, potions are drunk, monsters are fought
void game::FaceRoomCard(u32 Index, b32 Barehanded)
{
    Assert(Index < Room.size());

    card Card = Room[Index];
    Room.erase(Room.begin() + Index);

    LastCardWasPotion = false;

    switch(Card.Type)
    {
        case Card_Weapon:
        {
            EquipWeapon(Card);
        } break;
        case Card_Potion:
        {
            DrinkPotion(Card);
        } break;
        case Card_Monster:
        {
            FightMonster(Card, Barehanded);
        } break;
    }

    CardsFacedThisRoom++;
    CheckEndOfTurn();
}

// Weapons are binding: the previous weapon and the monsters it slew go to the discard pile
void game::EquipWeapon(card NewWeapon)
{
    for(card &Card : Weapon) Discard(Card);
    for(card &Card : SlainMonsters) Discard(Card);
    Weapon.clear();
    SlainMonsters.clear();

    NewWeapon.Pile = Pile_EquippedWeapon;
    NewWeapon.Position = WeaponPosition;
    Weapon.push_back(NewWeapon);

    Log(Info, "game::EquipWeapon() - Equipped a %d weapon", NewWeapon.Value);
}

// Heals up to MaxHealth. Only the first potion of each Room heals, the rest are simply discarded
void game::DrinkPotion(card Potion)
{
    if(!PotionUsedThisRoom)
    {
        i32 HealthBefore = PlayerHealth;
        PlayerHealth = glm::min(PlayerHealth + Potion.Value, MaxHealth);
        PotionUsedThisRoom = true;

        // Only a potion that healed counts for the full health win bonus
        LastCardWasPotion = true;
        LastPotionValue = Potion.Value;

        // No sound at full health, the potion healed nothing
        if(PlayerHealth > HealthBefore) Audio->Play(HealthPotionSound);

        Log(Info, "game::DrinkPotion() - Healed %d, Health: %d", Potion.Value, PlayerHealth);
    }
    else
    {
        Log(Info, "game::DrinkPotion() - Already drank a potion this Room, discarding it");
    }

    Discard(Potion);
}

// With the weapon: the damage is what the monster has over the weapon, and the monster goes on top of the weapon.
// Once the weapon slays a monster, it can only be used on monsters of lower value than the last one it slew.
// Barehanded: the full monster value is taken as damage and the monster is discarded
void game::FightMonster(card Monster, b32 Barehanded)
{
    b32 CanUseWeapon = !Weapon.empty() && (SlainMonsters.empty() || Monster.Value < SlainMonsters.back().Value);

    if(!Barehanded && CanUseWeapon)
    {
        i32 Damage = glm::max(Monster.Value - Weapon[0].Value, 0);
        PlayerHealth -= Damage;

        // Stacked on top of the weapon and staggered to the right, so the number of every card below stays visible
        f32 Count = (f32)(SlainMonsters.size() + 1);
        Monster.Pile = Pile_SlainMonsters;
        Monster.Position = WeaponPosition + glm::vec3(SlainMonsterStagger * Count, 0.0f, CardThickness * Count);
        SlainMonsters.push_back(Monster);

        Audio->Play(SlashSound);
        if(Damage > 0) Audio->Play(DamageSound);

        Log(Info, "game::FightMonster() - Slew a %d with a %d weapon, took %d damage, Health: %d", Monster.Value, Weapon[0].Value, Damage, PlayerHealth);
    }
    else
    {
        PlayerHealth -= Monster.Value;
        Discard(Monster);

        Audio->Play(PunchSound);
        Audio->Play(DamageSound);

        Log(Info, "game::FightMonster() - Fought a %d barehanded, Health: %d", Monster.Value, PlayerHealth);
    }
}

// Ends the game when the health runs out or the Dungeon is cleared, and deals the next Room after facing 3 cards
void game::CheckEndOfTurn()
{
    if(PlayerHealth <= 0)
    {
        // Score: every monster left in the Dungeon and in the Room is subtracted from the (zero or negative) health
        Score = PlayerHealth;
        std::vector<card> *RemainingPiles[] = {&Dungeon, &Room};
        for(std::vector<card> *Pile : RemainingPiles)
        {
            for(card &Card : *Pile)
            {
                if(Card.Type == Card_Monster) Score -= Card.Value;
            }
        }

        State = Game_Lost;
        Log(Info, "game::CheckEndOfTurn() - You died, Score: %d", Score);
        return;
    }

    if(Dungeon.empty() && Room.empty())
    {
        // Score: the remaining health. At full health, the value of a potion faced as the last card is added on top
        Score = PlayerHealth;
        if(PlayerHealth == MaxHealth && LastCardWasPotion)
        {
            Score += LastPotionValue;
        }

        State = Game_Won;
        Log(Info, "game::CheckEndOfTurn() - You made it through the Dungeon, Score: %d", Score);
        return;
    }

    if(CardsFacedThisRoom >= CardsToFacePerRoom)
    {
        // The turn is complete, the remaining card stays as part of the next Room
        AvoidedLastRoom = false;

        if(!Dungeon.empty())
        {
            DealRoom();
        }
        else
        {
            // Nothing left to deal, the last Room cards are faced as a new turn
            CardsFacedThisRoom = 0;
            PotionUsedThisRoom = false;
        }
    }
}

void game::DrawCards()
{
    std::vector<card> *Piles[] = {&Dungeon, &Room, &Weapon, &SlainMonsters, &DiscardPile};
    for(std::vector<card> *Pile : Piles)
    {
        for(const card &Card : *Pile)
        {
            Renderer->DrawCard(Card);
        }
    }
}

// Labels above each pile, in world space so they stay with the table. They float up and down together
void game::DrawLabels()
{
    f32 Time = (f32)SDL_GetTicks() / 1000.0f;
    f32 Wave = glm::sin(Time * LabelFloatSpeed); // -1 at the lowest point, +1 at the highest

    glm::vec3 AboveCard = glm::vec3(0.0f, CardSize.y * 0.5f + LabelGapAboveCard, 0.0f);
    glm::vec3 RoomCenter = glm::vec3((RoomCardPosition1.x + RoomCardPosition4.x) * 0.5f, RoomCardPosition1.y, RoomCardPosition1.z);

    char Text[64];
    snprintf(Text, sizeof(Text), "Dungeon (%zu)", Dungeon.size());
    DrawFloatingLabel(DungeonPosition + AboveCard, Text, Wave);

    snprintf(Text, sizeof(Text), "Discard (%zu)", DiscardPile.size());
    DrawFloatingLabel(DiscardPosition + AboveCard, Text, Wave);

    DrawFloatingLabel(WeaponPosition + AboveCard, "Weapon", Wave);
    DrawFloatingLabel(RoomCenter + AboveCard, "Room", Wave);
}

// Center is the center X and the top Y of the label at rest. Wave goes from -1 (lowest) to +1 (highest).
// Like a real shadow, the black shadow follows the label part of the way, so the gap grows as the label rises,
// and it gets a bit larger and lighter as the label moves away from the table
void game::DrawFloatingLabel(glm::vec3 Center, const char *Text, f32 Wave)
{
    f32 TextHeight = 1.2f; // cm
    bitmap_font *Font = AssetMgr->GetFont(MarcellusFont);

    // Shadow first, so the label is drawn on top of it
    f32 Height = (1.0f + Wave) * 0.5f; // 0 at the lowest point, 1 at the highest
    f32 ShadowHeight = TextHeight * glm::mix(1.0f, LabelShadowMaxScale, Height);
    f32 ShadowAlpha = glm::mix(LabelShadowAlphaLow, LabelShadowAlphaHigh, Height);
    f32 ShadowCenterY = Center.y - LabelShadowOffset - TextHeight * 0.5f + Wave * LabelFloatHeight * LabelShadowFollow; // Grows from its center in all directions
    glm::vec3 ShadowTopLeft = glm::vec3(Center.x - TextWidth(Font, ShadowHeight, Text) * 0.5f, ShadowCenterY + ShadowHeight * 0.5f, Center.z);
    Renderer->DrawText3D(MarcellusFont, ShadowTopLeft, ShadowHeight, Text, color{0.0f, 0.0f, 0.0f, ShadowAlpha});

    glm::vec3 TopLeft = glm::vec3(Center.x - TextWidth(Font, TextHeight, Text) * 0.5f, Center.y + Wave * LabelFloatHeight, Center.z);
    Renderer->DrawText3D(MarcellusFont, TopLeft, TextHeight, Text, color{1.0f, 1.0f, 1.0f, 1.0f});
}

// World text centered on the window X (the camera looks at X = 0), halfway between the bottom of the Room
// and the top of the lower row labels, so it scales with the table like the cards do
void game::DrawHealth()
{
    bitmap_font *Font = AssetMgr->GetFont(MarcellusFont);
    if(!Font) return;

    f32 Height = 1.7f; // cm, about 40 pixels at the starting window size

    char Text[128];
    snprintf(Text, sizeof(Text), "Health: %d", PlayerHealth);

    f32 RoomBottom = RoomCardPosition1.y - CardSize.y * 0.5f;
    f32 LowerRowTop = DungeonPosition.y + CardSize.y * 0.5f + LabelGapAboveCard;
    f32 MidY = (RoomBottom + LowerRowTop) * 0.5f;

    glm::vec3 TopLeft = glm::vec3(-TextWidth(Font, Height, Text) * 0.5f, MidY + Height * Font->Metrics.LineHeight * 0.5f, 0.0f);
    Renderer->DrawText3D(MarcellusFont, TopLeft, Height, Text, RED);
}

// Screen text sizes are in pixels for a ReferenceWindowHeight tall window, this scales them to the current window
f32 game::ScreenTextScale()
{
    return (f32)Renderer->ViewportHeight / ReferenceWindowHeight;
}

// What the player can do right now, and the score once the game is over
void game::UpdateActionHint()
{
    switch(State)
    {
        case Game_WaitingToStart:
        {
            snprintf(ActionHint, sizeof(ActionHint), "Click the Dungeon to enter");
        } break;
        case Game_Playing:
        {
            // NOTE: No trailing separator when the Room can't be avoided, it would push the centered text to the left
            b32 CanAvoid = (CardsFacedThisRoom == 0 && !AvoidedLastRoom && Room.size() == RoomSize && !Dungeon.empty());
            snprintf(ActionHint, sizeof(ActionHint), "Left click: face a card (monsters with your weapon)   Right click: fight barehanded%s",
                     CanAvoid ? "   Click the Dungeon: avoid the Room" : "");
        } break;
        case Game_Won:
        {
            snprintf(ActionHint, sizeof(ActionHint), "You made it through the Dungeon! Score: %d   Press R to play again", Score);
        } break;
        case Game_Lost:
        {
            snprintf(ActionHint, sizeof(ActionHint), "You died. Score: %d   Press R to play again", Score);
        } break;
    }
}

// Bottom center of the screen, centered on the text width
void game::DrawActionHint()
{
    bitmap_font *Font = AssetMgr->GetFont(MarcellusFont);
    if(!Font) return;

    f32 Scale = ScreenTextScale();
    f32 Size = 28.0f * Scale;
    f32 Margin = 20.0f * Scale;

    f32 X = ((f32)Renderer->ViewportWidth - TextWidth(Font, Size, ActionHint)) * 0.5f;
    f32 Y = (f32)Renderer->ViewportHeight - Margin - Size * Font->Metrics.LineHeight; // DrawText's Y is the top of the line

    Renderer->DrawText(MarcellusFont, X, Y, Size, ActionHint, color{1.0f, 1.0f, 1.0f, 1.0f});
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

// Cards are discarded face down on top of the discard pile
void game::Discard(card Card)
{
    Card.Pile = Pile_DiscardPile;
    Card.Position = DiscardPosition + glm::vec3(0.0f, 0.0f, CardThickness * (f32)DiscardPile.size());
    Card.Angle = glm::pi<f32>();
    Card.Axis = glm::vec3(0.0f, 1.0f, 0.0f);
    DiscardPile.push_back(Card);
}
