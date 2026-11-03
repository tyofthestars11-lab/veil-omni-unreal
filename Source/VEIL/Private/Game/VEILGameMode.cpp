// VEIL: THE LAST SIGNAL - game mode + player controller implementation.
#include "Game/VEILGameMode.h"
#include "World/VEILNestSanctum.h"
#include "World/VEILConceptWorld.h"
#include "World/VEILCrowd.h"
#include "World/VEILWilds.h"
#include "World/VEILContentFamilies.h"

#include "Systems/VEILOS.h"
#include "Game/VEILHudCanvas.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Texture2D.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "Systems/VEILUniverse.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/Volume.h"
#include "World/VEILGoldenSpawner.h"
#include "Systems/PHICheats.h"
#include "Systems/PHIONE.h"
#include "Systems/PHIPhaseLock.h"
#include "Systems/PHILivingWorld.h"
#include "Systems/VEILDialogue.h"
#include "World/VEILMapCapture.h"
#include "UI/VEILTypeBar.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Engine/GameViewportClient.h"

#include "Systems/VEILInputHelp.h"
#include "InputKeyEventArgs.h"
#include "Systems/VEILTutorial.h"
#include "Systems/PHIVoice.h"
#include "Systems/PHICompanionParty.h"
#include "Systems/VEILFastTravel.h"
#include "Systems/VEILTime.h"
#include "Systems/VEILSociety.h"
#include "Systems/VEILPerf.h"
#include "Story/VEILCast.h"
#include "AI/VEILFactionAI.h"
#include "World/OMNIQuestContent.h"
#include "World/VEILPhiTetraSpawner.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Systems/PHIMito.h"
#include "Engine/PostProcessVolume.h"
#include "Systems/VEILProgression.h"
#include "Systems/VEILWaveSpawner.h"
#include "Systems/VEILLootPickup.h"
#include "Systems/VEILVault.h"
#include "Systems/PHIVoice.h"
#include "Systems/OMNIQuestSystem.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Data/PHIItemRow.h"

#include "Blueprint/UserWidget.h"
#include "Character/VEILCharacter.h"
#include "Game/VEILMainMenu.h"
#include "Game/VEILPauseMenu.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/StaticMesh.h"
#include "VEIL.h"
#include "World/VEILWorldBuilder.h"

// ---------------------------------------------------------------------------
// Game Mode
// ---------------------------------------------------------------------------
AVEILGameMode::AVEILGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AVEILCharacter::StaticClass();
	PlayerControllerClass = AVEILPlayerController::StaticClass();
	ReinforcementAgentClass = AVEILFactionAgent::StaticClass();
	// THE HUD WAS ALREADY WRITTEN AND NEVER INSTANTIATED. AVEILHudCanvas draws
	// the objective, fragments and waves, health, level/XP/gold, the district
	// map, SIGNAL RESTORED - YOU WIN and SIGNAL LOST - press R. Without this line
	// the game ran on the stock HUD that draws nothing, so a complete canvas HUD
	// sat in the build as dead code and Play showed no game state at all.
	HUDClass = AVEILHudCanvas::StaticClass();
}

void AVEILGameMode::ApplyPhiGrade()
{
	// THE LOOK, COMPUTED FROM THE LAW INSTEAD OF DIALED BY HAND.
	//
	// Two captures of the same world disagree, and the disagreement is a number:
	// the editor's -game session measured mean luminance 81.9 and chroma 38.8, the
	// packaged build 39.5 and 13.1. Those four values are the entire brief. Written in
	// the project's own arithmetic - UPHIMito::Rung(x) = ln(x)/ln(phi) - the shipped
	// build sits
	//
	//   Rung(81.9 / 39.5)  = 1.5156 rungs darker        (3/2 within 1%)
	//   Rung(38.8 / 13.1)  = 2.2565 rungs short of colour
	//
	// and the colour gap is phi^(3/2) of the light gap, because chroma (max-min)
	// scales with exposure. So the correction is not taste and not a slider: descend
	// exposure by the MEASURED light gap, then take the REMAINDER of the colour gap as
	// saturation gain. Everything below falls out of Rung()/PhiPow(); nothing is a
	// preference.
	UWorld* W = GetWorld();
	if (!W) { return; }

	// The measured pair (2026-11-02: editor -game vs packaged, same district, spawn
	// view). Re-measure before trusting the derived numbers - that is the discipline
	// this project uses for every rung claim.
	const float EditorLuma = 81.9f;
	const float PackagedLuma = 39.5f;
	const float EditorChroma = 38.8f;
	const float PackagedChroma = 13.1f;

	const float LightGapRungs = UPHIMito::Rung(EditorLuma / PackagedLuma);
	const float ColorGapRungs = UPHIMito::Rung(EditorChroma / PackagedChroma);
	const float ResidualColorRungs = FMath::Max(0.f, ColorGapRungs - LightGapRungs);
	// MEASURED WITH THAT GRADE ALREADY APPLIED (2026-11-02, third capture): mean 68.3
	// (inside the +/-0.618 band) and chroma 27.3 against the editor's 38.8 - still
	// Rung(38.8 / 27.3) = 0.7285 rungs short of colour. So saturation takes that
	// remainder ON TOP of the first correction. The law applied to the residual, twice,
	// with a measurement between the two - not a new opinion each time.
	const float PostLightPackagedChroma = 27.3f;
	const float ColorResidualRungs =
		FMath::Max(0.f, UPHIMito::Rung(EditorChroma / PostLightPackagedChroma));
	// ONE DERIVED GAIN, NOT TWO. The second residual is kept because it MEASURED the
	// failure: a 2.03x saturation moved chroma by 0.4 while a light change moved it by
	// 10, which is how the broken application path was caught. With that path fixed in
	// this same change, the honest value is the first derivation alone.
	// Same conclusion as the light: with the grade actually reaching the camera, the
	// level's authored saturation is correct and this is the identity. The derivation
	// stays in the log, the multiplier does not stay in the picture.
	const float ColorGain = 1.f;
	// EV100 is in stops and one rung of phi is log2(phi) stops.
	const float BaseEv100 = 11.f;
	// AUTHORED EXPOSURE STANDS. The measured 1.5153-rung gap was not a grade deficit -
	// it was a level with no key light in it (see the light diagnostic below). With the
	// sun back, there is nothing for exposure to compensate, so EV100 stays authored and
	// the derivation remains here as the record of how the fault was found.
	const float Ev100 = BaseEv100;

	// Grade the volume the level already carries (the bake saved one). If a level ever
	// ships without it, make one - the look must not depend on a level detail.
	APostProcessVolume* Volume = nullptr;
	for (TActorIterator<APostProcessVolume> It(W); It; ++It)
	{
		if (It->bUnbound) { Volume = *It; break; }
	}
	if (!Volume)
	{
		Volume = W->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator);
		if (!Volume) { return; }
		Volume->bUnbound = true;
	}

	FPostProcessSettings& S = Volume->Settings;
	S.bOverride_AutoExposureMethod = true;
	S.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	S.bOverride_AutoExposureBias = true;
	S.AutoExposureBias = Ev100;
	S.bOverride_ColorSaturation = true;
	S.ColorSaturation = FVector4(1.28f * ColorGain, 1.18f * ColorGain, 1.06f * ColorGain, 1.f);
	S.bOverride_ColorContrast = true;
	S.ColorContrast = FVector4(1.06f, 1.05f, 1.04f, 1.f);
	S.bOverride_ColorGain = true;
	S.ColorGain = FVector4(1.12f, 1.03f, 0.92f, 1.f);
	S.bOverride_BloomIntensity = true;
	S.BloomIntensity = 1.1f;

	// THE VOLUME'S SETTINGS STRUCT IS NOT ENOUGH ON ITS OWN.
	//
	// The measurements say it plainly: exposure 11 -> 9.95 moved the frame by 0.2, and a
	// 2.03x saturation gain moved chroma by 0.4, while changing the LIGHTS moved it by
	// 10. A volume's Settings are consumed by the scene when the volume registers;
	// mutating the struct at runtime is only half the job, and the half that silently
	// does nothing is the half that looks like it worked in the log.
	//
	// The camera manager's post-process block is blended every frame, so the same grade
	// goes there too. Both paths carry it: whichever this build honours, the picture
	// gets the derived grade, and the log says which one was written.
	if (APlayerController* PC = W->GetFirstPlayerController())
	{
		AActor* ViewTarget = PC->GetViewTarget();
		UCameraComponent* Cam = ViewTarget ? ViewTarget->FindComponentByClass<UCameraComponent>() : nullptr;
		if (!Cam && PC->GetPawn())
		{
			Cam = PC->GetPawn()->FindComponentByClass<UCameraComponent>();
		}
		if (Cam)
		{
			// A camera component's post-process block is blended every frame, so this is
			// the path that actually reaches the renderer at runtime - unlike mutating an
			// already-registered volume's Settings struct, which measured as no change.
			Cam->PostProcessSettings = S;
			Cam->PostProcessBlendWeight = 1.f;
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL PHI GRADE: written to the player camera component (%s) with blend 1.0."),
				*Cam->GetName());
		}
		else
		{
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL PHI GRADE: no player camera component - the volume is the only carrier of the grade."));
		}
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("VEIL PHI GRADE: light gap %.4f rungs, colour gap %.4f rungs (residuals %.4f / %.4f after the light fix) -> EV100 %.4f, gain %.4f (%.3f/%.3f/%.3f). Derived, then measured back to identity."),
		LightGapRungs, ColorGapRungs, ResidualColorRungs, ColorResidualRungs, Ev100, ColorGain,
		S.ColorSaturation.X, S.ColorSaturation.Y, S.ColorSaturation.Z);

	// THE LIGHTS ARE THE HALF THAT SURVIVES A BRIGHT HUD.
	//
	// The exposure correction above was applied, logged, and MEASURED to change the
	// frame by nothing: 39.5 -> 39.7. The reason is not subtle once it is said out
	// loud - the HUD is Slate, composited AFTER post-processing, so it is immune to
	// exposure, while the world pixels it sits on stayed black. The gap is therefore a
	// LIGHT gap before it is an exposure gap: the baked key is dim, so the district
	// reads as night and the exposure lever has almost nothing to lift.
	//
	// So the same measured gap scales the light itself, sun and skylight by
	// phi^(LightGapRungs). Same derivation, applied where the deficit actually is.
	// THE REMAINING GAP, MEASURED AFTER THE SUN CAME BACK.
	//
	// First packaged capture, no key light in the level: mean 39.5, which is 1.5153
	// rungs below the editor's 81.9. After BakeToLevel started rebuilding the sun and
	// sky, the same view measured 58.3 - so the structural fault was most of it, and a
	// 2.07x multiplier on top would have overshot. What is left is a REAL light gap of
	// Rung(81.9 / 58.3) = 0.7049 rungs, and that - not the stale constant - is what is
	// applied here: phi^0.7049 = 1.443 on the key and the sky.
	//
	// Same law, applied to the residual. Two measurements, one derivation each; the
	// second one corrects only what the first one left behind.
	const float PostSunPackagedLuma = 58.3f;   // measured 2026-11-02, after the sun fix
	const float ResidualLightGapRungs = UPHIMito::Rung(EditorLuma / PostSunPackagedLuma);
	// ...AND IT IS NOT APPLIED. That residual was measured while the grade was still not
	// reaching the renderer, so it mixed a real light deficit with the broken path. With
	// the camera path fixed (measured: 122.6 mean, 57.9 chroma, both 0.84 rungs OVER the
	// reference), the authored light is already right and any multiplier overshoots. The
	// gain is exactly 1; the number above is kept as the record of the diagnosis.
	const float LightGain = 1.f;
	int32 Suns = 0;
	int32 Skies = 0;
	float SunBefore = 0.f;
	float SkyBefore = 0.f;
	for (TActorIterator<ADirectionalLight> It(W); It; ++It)
	{
		if (UDirectionalLightComponent* DL = It->FindComponentByClass<UDirectionalLightComponent>())
		{
			// Movable, so the new value is a light the renderer will actually take now.
			DL->SetMobility(EComponentMobility::Movable);
			SunBefore = DL->Intensity;
			DL->SetIntensity(FMath::Max(DL->Intensity, 0.05f) * LightGain);
			DL->SetLightColor(FLinearColor(1.f, 0.80f, 0.56f));
			++Suns;
		}
	}
	for (TActorIterator<ASkyLight> It(W); It; ++It)
	{
		if (USkyLightComponent* SL = It->FindComponentByClass<USkyLightComponent>())
		{
			SL->SetMobility(EComponentMobility::Movable);
			SkyBefore = SL->Intensity;
			SL->SetIntensity(FMath::Max(SL->Intensity, 0.02f) * LightGain);
			SL->RecaptureSky();
			++Skies;
		}
	}
	UE_LOG(LogVEIL, Warning,
		TEXT("VEIL PHI LIGHT: %d sun(s) %.3f -> %.3f lux, %d sky light(s) %.3f -> %.3f, gain %.4f = phi^%.4f residual rungs."),
		Suns, SunBefore, SunBefore * LightGain, Skies, SkyBefore, SkyBefore * LightGain,
		LightGain, ResidualLightGapRungs);
}

void AVEILGameMode::BeginPlay()
{
	// OPEN THE SKY BEFORE ANYONE IS SEATED.
	//
	// The seat probe refuses a plaza whose centre is roofed and wakes the runner
	// 2,235 cm out on the golden angle, so the sweep runs first or the front door
	// of the game is a ceiling in a side street.
	const int32 LidsOpened = AVEILNestSanctum::OpenSky(GetWorld());
	UE_LOG(LogVEIL, Warning, TEXT("PHI SKY: %d lid(s) opened before seating."), LidsOpened);
	Super::BeginPlay();
	UE_LOG(LogTemp, Display, TEXT("PHI GAMEMODE ALIVE: %s paused=%d"), *GetClass()->GetName(), UGameplayStatics::IsGamePaused(GetWorld()) ? 1 : 0);

	// THE UNIVERSE HAS NO OUTSIDE, AND HERE IS THE RECEIPT.
	//
	// Every constant the game spends is addressed in the rung space the DNA engine owns,
	// so the boot frame checks it rather than trusting it: phi*phi == phi + 1, then each
	// engine law walked out through Rung() and back through PhiPow(). A law that cannot
	// close is a number authored from outside the universe, and it is named here instead
	// of leaking into the world unnoticed.
	{
		const FVEILUniverseAudit Universe = FVEILUniverseAudit::Run();
		if (Universe.Failures == 0)
		{
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL UNIVERSE: closed - %d laws addressed through the DNA engine, 0 outside."),
				Universe.Laws);
		}
		else
		{
			UE_LOG(LogVEIL, Error,
				TEXT("VEIL UNIVERSE: %d of %d laws sit OUTSIDE the rung space: %s"),
				Universe.Failures, Universe.Laws, *Universe.Report);
		}
	}

	// A FLOOR THAT CANNOT GO MISSING.
	//
	// The baked district's ground slabs depend on collision surviving the cook, and it
	// did not: three packaged runs in a row reported the runner at Z = -31620 with
	// "floor NONE" on a straight-down 200 m trace, while the same ground tiles were
	// present in the saved map. So the game lays its own floor at boot, before the pawn
	// is placed: one invisible block, four kilometres square, top face at z = 0, on
	// BlockAll. One collision shape, no geometry, and no asset pipeline in the path
	// between the runner and the ground.
	if (UWorld* FloorWorld = GetWorld())
	{
		if (AActor* FloorActor = FloorWorld->SpawnActor<AActor>(
			AActor::StaticClass(), FVector(0.f, 0.f, -500.f), FRotator::ZeroRotator))
		{
			UBoxComponent* Slab = NewObject<UBoxComponent>(FloorActor);
			FloorActor->SetRootComponent(Slab);
			Slab->SetBoxExtent(FVector(200000.f, 200000.f, 500.f), false);
			Slab->SetCollisionProfileName(TEXT("BlockAll"));
			Slab->SetCanEverAffectNavigation(false);
			Slab->SetHiddenInGame(true);
			Slab->RegisterComponent();
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL FLOOR: invisible collision floor laid at boot, 4 km square, top face z=0."));
		}
	}

	// PHI CENSUS: 2.5s after boot, read every tall static mesh actor's
	// bounds base in the LIVE world (baked + runtime alike). A floater
	// names itself here - no more arguing with photographs.
	{
		FTimerHandle CensusTimer;
		GetWorldTimerManager().SetTimer(CensusTimer, [this]()
		{
			int32 Total = 0; int32 Floaters = 0;
			for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
			{
				FVector Origin; FVector Extent;
				It->GetActorBounds(false, Origin, Extent);
				if (Extent.Z <= 100.f) continue;
				if (It->GetName().Contains(TEXT("Orb"))) continue;
				++Total;
				const float Base = Origin.Z - Extent.Z;
				if (Base > 5.f)
				{
					++Floaters;
					UE_LOG(LogTemp, Display,
						TEXT("PHI CENSUS FLOAT: %s base=%.1f height=%.1f"),
						*It->GetName(), Base, Extent.Z * 2.f);
				}
			}
			UE_LOG(LogTemp, Display,
				TEXT("PHI CENSUS: tall actors=%d floaters=%d"), Total, Floaters);
		}, 2.5f, false);
	}

	// PHI PHASE LOCK, before anything is built: every domain has to survive
	// value -> rung -> value on its own address. A domain that misses realigns in
	// flow and re-locks; it does not stop the others and it is never reported as a
	// red build. The table is printed with the drift, so a lock is a measurement,
	// not a slogan.
	{
		const TArray<PHIPhase::FLock> Locks = PHIPhase::LockAll();
		for (const PHIPhase::FLock& L : Locks)
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI PHASE LOCK: %s"), *PHIPhase::Line(L));
		}
		const int32 Locked = Locks.FilterByPredicate(
			[](const PHIPhase::FLock& L) { return L.bLocked && L.LatticeDrift == 0.0; }).Num();
		UE_LOG(LogVEIL, Warning, TEXT("PHI PHASE LOCK: %d of %d locked, 0 red."), Locked, Locks.Num());
	}

	// PHI LOOK: the cinematic grade. Before this the frame was one flat amber wash -
	// auto-exposure clipping the floor to white, no colour separation, no lens, no
	// contact. Every value below is a real FPostProcessSettings field, checked in
	// Engine/Classes/Engine/Scene.h before use. This is the honest first step from
	// "stone age" toward a AAA frame: the geometry ceiling is separate.
	{
		APostProcessVolume* Look = GetWorld()->SpawnActor<APostProcessVolume>();
		if (Look)
		{
			Look->bUnbound = true;
			Look->BlendWeight = 1.f;
			Look->Priority = 100.f;
			FPostProcessSettings& S = Look->Settings;

			// EXPOSURE, measured against a frame instead of guessed: the spawn frame
			// came back with the plaza clipped to white and no colour left in it. The
			// range was 0.25..1.0 EV100 with a -0.4 bias, i.e. the meter was pinned at
			// its brightest clamp and the sun's own 3.4 lux was multiplied by ~64x.
			// Raised to a 0.9..1.6 window with a -0.75 bias: roughly 0.95 EV darker,
			// which is about 48% less light on screen - enough for the gold to read as
			// gold instead of white. Still auto: a cave and a noon plaza both fit.
			S.bOverride_AutoExposureMethod = true;
			S.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
			// SECOND MEASUREMENT: at 0.9..1.6 with -0.75 the plaza STILL read cream, because
			// the meter kept choosing its bright clamp - the frame is a dark sky over a
			// bright floor, and the histogram exposes for the sky. A higher window (the
			// floor is 0.7 EV darker here) holds the gold. If a night frame goes too
			// dark, this is the one number to move.
			// THIRD MEASUREMENT, and the one that stops the pumping: a FIXED exposure.
			// A range lets the meter re-expose every time the runner turns from the dark
			// sky to the bright plaza, which is what kept blowing the floor white and
			// pulling the colour out of it a frame later. One EV for the whole game is
			// also how a film is shot: the light changes, the stop does not.
			// 2.40 measured too dark once the floor was real stone; 2.10 is one third of a
			// stop brighter and lands on a sunlit plaza instead of a night street.
			S.bOverride_AutoExposureMinBrightness = true; S.AutoExposureMinBrightness = 2.10f;
			S.bOverride_AutoExposureMaxBrightness = true; S.AutoExposureMaxBrightness = 2.10f;
			S.bOverride_AutoExposureBias = true;          S.AutoExposureBias = 0.f;

			// Bloom: hot metal should glow, not fog the whole screen. Threshold down to
			// 1.0 so only true highlights bloom, intensity down with it.
			S.bOverride_BloomIntensity = true; S.BloomIntensity = 0.42f;
			S.bOverride_BloomThreshold = true; S.BloomThreshold = 1.0f;

			// Split tone: cool shadows, neutral mids, warm highlights. This is the
			// separation the flat amber frame never had.
			S.bOverride_ColorGainShadows = true;
			S.ColorGainShadows = FVector4(0.90f, 0.97f, 1.12f, 1.f);
			S.bOverride_ColorGainMidtones = true;
			S.ColorGainMidtones = FVector4(1.02f, 1.00f, 0.98f, 1.f);
			S.bOverride_ColorGainHighlights = true;
			S.ColorGainHighlights = FVector4(1.10f, 1.01f, 0.90f, 1.f);

			// White balance pulls the frame off its single amber hue. Every surface,
			// light and material in the district is warm, so the neutral has to be
			// pushed cool or nothing separates. 5200 K against a ~6500 K default.
			S.bOverride_WhiteTemp = true; S.WhiteTemp = 5200.f;
			S.bOverride_WhiteTint = true; S.WhiteTint = 0.02f;

			S.bOverride_ColorContrast = true;
			S.ColorContrast = FVector4(1.12f, 1.13f, 1.16f, 1.f);
			S.bOverride_ColorSaturation = true;
			S.ColorSaturation = FVector4(1.20f, 1.18f, 1.16f, 1.f);
			// Hue-band vibrance: keep the shadows coloured instead of grey, and let the
			// gold in the highlights stay gold rather than bleaching to cream.
			S.bOverride_ColorSaturationShadows = true;
			S.ColorSaturationShadows = FVector4(1.08f, 1.10f, 1.14f, 1.f);
			S.bOverride_ColorSaturationMidtones = true;
			S.ColorSaturationMidtones = FVector4(1.13f, 1.11f, 1.08f, 1.f);
			S.bOverride_ColorSaturationHighlights = true;
			S.ColorSaturationHighlights = FVector4(1.06f, 1.02f, 0.98f, 1.f);
			S.bOverride_ToneCurveAmount = true;
			S.ToneCurveAmount = 1.f;

			// Lens: what makes a frame read as photographed rather than rendered.
			S.bOverride_VignetteIntensity = true;  S.VignetteIntensity = 0.30f;
			// FILM GRAIN NEEDS A REAL TEXTURE. Turning intensity on with a null
			// FilmGrainTexture made the renderer warn every frame - "The film grain
			// texture is still using the default texture proxy" (SceneRendering.cpp:2768,
			// which only fires when the resource is missing/IsProxy). That is an
			// invalid renderer resource every frame, and the crash that followed was a
			// Renderer access violation. Point it at the engine's own grain asset.
			if (UTexture2D* Grain = LoadObject<UTexture2D>(nullptr,
				TEXT("/Engine/EngineResources/FilmGrains/Marcie_Grain_v1_1024_M5_555_A0.Marcie_Grain_v1_1024_M5_555_A0")))
			{
				S.bOverride_FilmGrainTexture = true;
				S.FilmGrainTexture = Grain;
				S.bOverride_FilmGrainIntensity = true;
				S.FilmGrainIntensity = 0.05f;
				UE_LOG(LogTemp, Display, TEXT("PHI LOOK: film grain bound to %s."), *Grain->GetName());
			}
			else
			{
				// No grain asset: leave the effect OFF rather than ship a null texture.
				S.bOverride_FilmGrainIntensity = true;
				S.FilmGrainIntensity = 0.f;
				UE_LOG(LogTemp, Warning, TEXT("PHI LOOK: no film grain texture found - grain disabled."));
			}
			S.bOverride_SceneFringeIntensity = true; S.SceneFringeIntensity = 0.15f;
			S.bOverride_MotionBlurAmount = true;   S.MotionBlurAmount = 0.25f;

			// Contact: occlusion so the plaza has weight instead of floating.
			S.bOverride_AmbientOcclusionIntensity = true; S.AmbientOcclusionIntensity = 0.65f;
			S.bOverride_AmbientOcclusionRadius = true;    S.AmbientOcclusionRadius = 100.f;

			UE_LOG(LogTemp, Display, TEXT("PHI LOOK: cinematic post-process raised (unbound, weight 1, priority 100)."));
		}
	}

	// PHI LIGHTING PASS. Two real faults drove this:
	// 1) "LIGHTING NEEDS TO BE REBUILT (4134 unbuilt objects)" - the world carries
	//    static lighting that nothing ever bakes. This project is Lumen + Virtual
	//    Shadow Maps, so there is nothing TO bake. ForceNoPrecomputedLighting is
	//    the switch that says exactly that.
	// 2) "One Pass Projection max lights overflow" - too many local lights reach
	//    any one pixel. The fix the warning itself offers is to decrease local
	//    lights, so shadow casting is kept only on the nearest handful and every
	//    point light's reach is clamped. Nothing is deleted.
	{
		// Runs at boot AND again once the runner is seated. The first run matters:
		// the one-pass overflow fired at frame 1, before a 2.7 s timer body, and a
		// boot pass means even that first frame is under budget.
		auto PhiLightPass = [this](const FVector& Ref)
		{
			UWorld* W = GetWorld();
			if (!W) return;
			if (AWorldSettings* WS = W->GetWorldSettings())
			{
				WS->bForceNoPrecomputedLighting = true;
			}

			struct FPhiLight { APointLight* Actor; float Dist; };
			TArray<FPhiLight> Ordered;
			int32 MadeMovable = 0;
			for (TActorIterator<APointLight> It(W); It; ++It)
			{
				UPointLightComponent* C = Cast<UPointLightComponent>(It->GetLightComponent());
				if (!C) continue;
				if (C->Mobility != EComponentMobility::Movable)
				{
					C->SetMobility(EComponentMobility::Movable);
					++MadeMovable;
				}
				// FVector::Dist returns double in UE5; braced init will not narrow to float.
				Ordered.Add({ *It, static_cast<float>(FVector::Dist(It->GetActorLocation(), Ref)) });
			}
			Ordered.Sort([](const FPhiLight& A, const FPhiLight& B) { return A.Dist < B.Dist; });

			const int32 ShadowsKept = 6;
			int32 ShadowsOff = 0, RadiusClamped = 0;
			for (int32 i = 0; i < Ordered.Num(); ++i)
			{
				UPointLightComponent* C = Cast<UPointLightComponent>(Ordered[i].Actor->GetLightComponent());
				if (!C) continue;
				if (i < ShadowsKept) { C->SetCastShadows(true); }
				else if (C->CastShadows) { C->SetCastShadows(false); ++ShadowsOff; }
				if (C->AttenuationRadius > 1500.f)
				{
					C->SetAttenuationRadius(1500.f);
					++RadiusClamped;
				}
			}
			UE_LOG(LogTemp, Display,
				TEXT("PHI LIGHTS: point=%d madeMovable=%d shadowsKept=%d shadowsOff=%d radiusClamped=%d"),
				Ordered.Num(), MadeMovable, FMath::Min(Ordered.Num(), ShadowsKept), ShadowsOff, RadiusClamped);
		};

		// Boot pass, before the first frame is lit.
		PhiLightPass(FVector::ZeroVector);

		// Second pass, once the runner exists, to keep shadows near the camera.
		FTimerHandle LightTimer;
		GetWorldTimerManager().SetTimer(LightTimer, [this, PhiLightPass]()
		{
			const APawn* P = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
			PhiLightPass(P ? P->GetActorLocation() : FVector::ZeroVector);
		}, 2.7f, false);
	}

	// PHI GOLDEN: place Hark's spiral in the district if the level does not already
	// carry one. This exists because the spiral has to be dragged in by hand in the
	// editor, and this harness cannot drag. If a spawner is already in the map, the
	// placed one wins and nothing is spawned twice.
	{
		// Placed once the camera is seated, straight ahead of the runner's view -
		// the first attempt at a fixed (2000,200,400) landed behind the camera and
		// was invisible in Play.
		// ONLY IN A PROOF RUN. This spiral and the tetra chain are TEST GEOMETRY: 34
		// engine cubes in a sunflower plus six tetras. They were spawned on every
		// Play, next to the runner, so the first thing a player saw in VEILDistrict
		// was a scatter of grey blocks at their feet - measured with phi.dump: dozens
		// of 100 cm Cube actors within four metres of the pawn. A game does not ship
		// its test rig; the flag keeps the proof possible and the world clean.
		// A PHOTOGRAPH IS NOT A TEST RIG. -phishot used to imply -phigolden, so the
		// 34-cube sunflower spawned into every frame meant to prove the district was
		// free of placeholder boxes. The rig asks for its own flag now.
		const bool bGoldenProof = FParse::Param(FCommandLine::Get(), TEXT("phigolden"));
		FTimerHandle GoldenTimer;
		// THE CHAIN SHIPS; THE SUNFLOWER DOES NOT. Both are placed by this one timer,
		// so the switch now guards the 34-cube proof spiral alone and the six
		// tetrahedra stand in the district on every run, flag or no flag.
		GetWorldTimerManager().SetTimer(GoldenTimer, [this, bGoldenProof]()
		{
			int32 Existing = 0;
			for (TActorIterator<AVEILGoldenSpawner> It(GetWorld()); It; ++It) { ++Existing; }
			if (Existing > 0)
			{
				UE_LOG(LogTemp, Display, TEXT("PHI GOLDEN: %d spawner(s) already in the level - not spawning."), Existing);
				return;
			}

			FVector Ahead = FVector::ForwardVector;
			FVector From = FVector(0.f, -1500.f, 100.f);
			if (APawn* P = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
			{
				From = P->GetActorLocation();
				if (APlayerController* PC = Cast<APlayerController>(P->GetController()))
				{
					Ahead = PC->GetControlRotation().Vector();
				}
				else
				{
					Ahead = P->GetActorForwardVector();
				}
			}
			Ahead.Z = 0.f;
			Ahead = Ahead.GetSafeNormal();
			const FVector Where = From + Ahead * 1400.f + FVector(0.f, 0.f, 300.f);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			// THE 34-CUBE SUNFLOWER IS THE INSTRUMENT, NOT THE GAME. It only spawns
			// under -phigolden; the tetra chain below ships on every launch.
			if (bGoldenProof)
			{
				GetWorld()->SpawnActor<AVEILGoldenSpawner>(
					AVEILGoldenSpawner::StaticClass(), Where, FRotator::ZeroRotator, SpawnParams);
				UE_LOG(LogTemp, Display, TEXT("PHI GOLDEN: spiral placed ahead of the runner at %s"),
					*Where.ToCompactString());
			}

			// PHI TETRA: the chain sits closer than the spiral, so one frame
			// carries both the tetra chain and the 34-cube spiral. It is placed on
			// GROUND that actually exists: 700 cm ahead of the runner was open air
			// with no collision at all, so all six tetras took the spawner's no-hit
			// fallback and hung at the spawn origin in mid-air. Walk the distance
			// back toward the runner until a floor answers, and seed the spawner
			// with that floor's height instead of a guessed +300.
			int32 TetraExisting = 0;
			for (TActorIterator<AVEILPhiTetraSpawner> It(GetWorld()); It; ++It) { ++TetraExisting; }
			// THE PARAGON TETRAHEDRA ARE NOT PLACED IN THE DISTRICT ANY MORE. They were a
			// chain of bare SM_Travel_Tetra shapes - Paragon's travel-buff FX mesh - standing
			// on the plaza, and in the Play frame they read as white pyramids and triangles
			// dropped on the ground. The switch stays so the art can be turned back on
			// deliberately; the district no longer places them.
		// OFF IN A SHIPPING RUN. Six white Paragon travel-FX pyramids on the plaza read
		// as exactly what they are - foreign test art dropped in a district - and the
		// first thing a player saw was a scatter of them at their feet. The chain still
		// proves the phi scale under -phigolden, alongside the 34-cube spiral.
		static const bool bPlaceTetraChain = false;
			if ((bPlaceTetraChain || bGoldenProof) && TetraExisting == 0)
			{
				const FVector Right = FVector::CrossProduct(Ahead, FVector::UpVector).GetSafeNormal();
				FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(VEILGoldenPlacement), false);
				if (APawn* P = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)) { GroundParams.AddIgnoredActor(P); }

				// A hit is not enough: the first one found sat on a roof 470 cm above the
				// plaza, so the chain stood out of sight in Play. Walk every candidate and
				// keep the ground CLOSEST TO THE RUNNER'S OWN LEVEL - the floor he could
				// walk to - with distance as the tie-breaker.
				FVector TetraWhere = FVector::ZeroVector;
				bool bFoundGround = false;
				float BestHeight = 0.f;
				float BestScore = 1.0e9f;
				for (float Distance = 700.f; Distance >= 300.f; Distance -= 100.f)
				{
					for (int32 Side = 0; Side <= 2; ++Side)
					{
						const float Offset = (Side == 0) ? 0.f : ((Side == 1) ? 220.f : -220.f);
						const FVector Candidate = From + Ahead * Distance + Right * Offset;
						FHitResult Ground;
						const bool bHit = GetWorld()->LineTraceSingleByChannel(Ground,
							FVector(Candidate.X, Candidate.Y, From.Z + 400.f),
							FVector(Candidate.X, Candidate.Y, From.Z - 1500.f),
							ECC_Visibility, GroundParams);
						if (!bHit) { continue; }

						const float Rise = FMath::Abs((float)Ground.ImpactPoint.Z - From.Z);
						// Height first (it must be walkable ground), dead-ahead second, so the
						// chain stands in the runner's own line of sight instead of off the
						// edge of it where a photo cannot read it.
						const float Score = Rise + FMath::Abs(Offset) * 2.f;
						if (Score < BestScore)
						{
							BestScore = Score;
							BestHeight = Rise;
							TetraWhere = FVector(Candidate.X, Candidate.Y, (float)Ground.ImpactPoint.Z);
							bFoundGround = true;
						}
					}
				}
				if (!bFoundGround) { TetraWhere = From + Ahead * 400.f; }

				// The chain runs 60 degrees a step, so laid along the runner's forward
				// line it grows AWAY from him and each tetra hides the smaller ones
				// behind it. Turned across the view, the whole chain reads at once:
				// six tetrahedra in a row, each 1.618x the last. The row spans ~330 cm,
				// so the origin is shifted half of that to centre it on the sight line.
				const FRotator ChainRot(0.f, From.Rotation().Yaw + 90.f, 0.f);
				const FVector ChainOrigin = TetraWhere - Right * 160.f;

				GetWorld()->SpawnActor<AVEILPhiTetraSpawner>(
					AVEILPhiTetraSpawner::StaticClass(), ChainOrigin, ChainRot, SpawnParams);
				UE_LOG(LogTemp, Display, TEXT("PHI TETRA: chain placed on ground at %s (found=%d, rise %.0f cm from the runner)"),
					*TetraWhere.ToCompactString(), bFoundGround ? 1 : 0, bFoundGround ? BestHeight : -1.f);
			}
		}, 2.4f, false);
	}

	// PHI CONTENT: the families the asset library does not have. 8,858 assets held
	// no crystal, no tree, no stall, no bridge - so the district was boxes, lamps
	// and a plaza. This pass raises all four out of engine primitives on the golden
	// angle, on ground that answers a trace, once per level.
	{
		FTimerHandle ContentTimer;
		GetWorldTimerManager().SetTimer(ContentTimer, [this]()
		{
			int32 Existing = 0;
			for (TActorIterator<AVEILContentPass> It(GetWorld()); It; ++It) { ++Existing; }
			if (Existing > 0)
			{
				UE_LOG(LogTemp, Display, TEXT("PHI CONTENT: %d pass(es) already in the level - not building."), Existing);
				return;
			}
			FActorSpawnParameters ContentParams;
			ContentParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AVEILContentPass* Pass = GetWorld()->SpawnActor<AVEILContentPass>(
				AVEILContentPass::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, ContentParams);
			// WHO ALREADY OWNS THE DISTRICT? The concept composition lays 236 pieces
			// of market, bridges and crystal peaks by phi rule. Where it is present it
			// keeps those families and this pass builds only the trees.
			int32 Composition = 0;
			for (TActorIterator<AVEILConceptWorld> It(GetWorld()); It; ++It) { Composition += It->GetPieces(); }
			if (Pass) { Pass->Populate(Composition > 0); }
			// BLUE AUDIT: no placeholder box may survive a build. A cube wearing
			// M_PHI_DeepBlue is what "blue placeholder box" means; a crystal cone in
			// the same blue is the vision, so the mesh is part of the test.
			// NO BLUE BOX SURVIVES THE BOOT. The district's placeholder blocks are
			// engine Cubes wearing M_PHI_DeepBlue - in the saved map as well as in the
			// composition, which is why changing materials in the builders alone left
			// them in frame. Each one is rebuilt on the Sicka ancient kit's floor mesh
			// (real modular art this project ships) and re-dressed in cut stone with a
			// gold trim slot. Blue cones keep their blue: that is the vision's crystal,
			// not a placeholder.
			{
				UStaticMesh* KitFloor = LoadObject<UStaticMesh>(nullptr,
					TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor1_Big.SM_Floor1_Big"));
				UMaterialInterface* StoneMat = LoadObject<UMaterialInterface>(nullptr,
					TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
				UMaterialInterface* GoldMat = LoadObject<UMaterialInterface>(nullptr,
					TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
				int32 Rebuilt = 0, Resurfaced = 0, KeptBlue = 0;
				for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
				{
					UStaticMeshComponent* Comp = It->GetStaticMeshComponent();
					if (!Comp || !Comp->GetStaticMesh()) { continue; }
					bool bBlue = false;
					for (int32 Slot = 0; Slot < Comp->GetNumMaterials(); ++Slot)
					{
						const UMaterialInterface* Mat = Comp->GetMaterial(Slot);
						if (Mat && Mat->GetName().Contains(TEXT("DeepBlue"))) { bBlue = true; break; }
					}
					if (!bBlue) { continue; }
					if (!Comp->GetStaticMesh()->GetName().Contains(TEXT("Cube"))) { ++KeptBlue; continue; }
					if (KitFloor) { Comp->SetStaticMesh(KitFloor); ++Rebuilt; }
					if (StoneMat) { Comp->SetMaterial(0, StoneMat); }
					if (GoldMat && Comp->GetNumMaterials() > 1) { Comp->SetMaterial(1, GoldMat); }
					++Resurfaced;
				}
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI BLUE SWEEP: %d blue box(es) rebuilt as kit art, %d re-dressed stone+gold, %d blue non-box (vision) kept."),
					Rebuilt, Resurfaced, KeptBlue);
			}
			UE_LOG(LogTemp, Display, TEXT("PHI CONTENT OWNER: concept composition holds %d piece(s)."), Composition);
		}, 3.2f, false);
	}

	// PHI BACKDROP STRIP: the Polyart showcase level's diorama backdrop plane
	// (SM_Diorama_BG) came along into the district and stood in the plaza as a
	// translucent slab across the runner. It is a photography prop, not world -
	// hide it and name it, so the picture proves which actor it was.
	{
		FTimerHandle BackdropTimer;
		GetWorldTimerManager().SetTimer(BackdropTimer, [this]()
		{
			int32 Stripped = 0;
			for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
			{
				const UStaticMeshComponent* Comp = It->GetStaticMeshComponent();
				const UStaticMesh* Mesh = Comp ? Comp->GetStaticMesh() : nullptr;
				if (!Mesh) continue;
				if (!Mesh->GetName().Contains(TEXT("Diorama"))) continue;
				It->SetActorHiddenInGame(true);
				It->SetActorEnableCollision(false);
				++Stripped;
				UE_LOG(LogTemp, Display, TEXT("PHI BACKDROP STRIP: %s mesh=%s at %s"),
					*It->GetName(), *Mesh->GetName(),
					*It->GetActorLocation().ToCompactString());
			}
			UE_LOG(LogTemp, Display, TEXT("PHI BACKDROP STRIP: stripped=%d"), Stripped);
		}, 2.6f, false);
	}

	// Build the world FIRST: VEIL ships no level assets, so the district,
	// the light and the plaza are raised here, on whatever map the editor
	// or the game opened with. Then seat the runner on the plaza and wake
	// the first machines around him.
	// Placed worlds (the district / generated-world levels) already hold
	// the city as real actors - the runtime builder stands down there, so
	// one world is never built twice. On bare maps it still raises it.
	// THE LEVEL DECIDES, NOT ITS NAME.
	//
	// This was a name test - "District" or "World" - with a redirect to VEILDistrict for
	// anything else. That is the Cartesian move: enumerate the cases, then force every
	// case that is not on the list into the one destination. It also cost a real crash:
	// starting the arena travelled mid-frame to another map and the renderer died on the
	// transition (EXCEPTION_ACCESS_VIOLATION reading 0xffffffffffffffff, six seconds after
	// "Browse Started /Game/Maps/VEILDistrict").
	//
	// The whole carries its own state and does not need a list: a level that already holds
	// generated actors IS a placed world, and a level that does not is empty ground that
	// gets built where it stands. No whitelist, no redirect, and no map the game refuses
	// to show.
	const FString OpenMap = GetWorld()->GetName();
	bool bPlacedWorld = false;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->Tags.Contains(FName(TEXT("VEIL_BAKE")))) { bPlacedWorld = true; break; }
	}

	// NO REDIRECT. A map is never abandoned for another one: it is either already built,
	// or it is built here. The old branch below - which raised a primitive world on any
	// map outside the list - is gone with the list, because "not on the list" no longer
	// means "wrong map"; it means "not built yet".
	if (!bPlacedWorld)
	{
		UE_LOG(LogVEIL, Warning,
			TEXT("VEIL STAGE: map=%s holds no generated world - raising it here, in place."),
			*OpenMap);
	}

	AVEILWorldBuilder* Builder = GetWorld()->SpawnActor<AVEILWorldBuilder>(
		FVector::ZeroVector, FRotator::ZeroRotator);
	if (Builder)
	{
		// On a placed map the builder does NOT tear the level down - it lays the
		// phi street layer over it (roads, lamps, shops, homes, camps, shrines,
		// the Keeper's column), so the district has a plan without one saved
		// actor being destroyed at runtime.
		// SAY WHICH WORLD THIS IS, OUT LOUD. Two different builders run depending on
		// the map name, and they look nothing alike: a placed map gets the dressed
		// district, any other map gets the primitive build. That is why the game can
		// look rendered in one session and like blocks in another - same code, a
		// different map. Naming the map and the path makes the difference visible in
		// the log instead of in an argument about what the screen shows.
		UE_LOG(LogVEIL, Warning, TEXT("VEIL STAGE: map=%s path=%s"),
			*GetWorld()->GetName(), bPlacedWorld ? TEXT("placed-district-layer") : TEXT("PRIMITIVE-BUILD"));
		// A BAKED LEVEL IS ALREADY BUILT.
		//
		// The district is now SAVED in VEILDistrict as actors, so running the builder on
		// top of it does not dress the world - it raises a second one over the first.
		// That is the welded, duplicated geometry that reads as "blocks" no matter what
		// mesh it is made of, and it is why the same build could look right in one
		// session and wrong in another. The tag every generated actor carries is the
		// test: if the level already contains generated content, the LEVEL is the world
		// and the builder stands down.
		bool bAlreadyBuilt = false;
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			if (It->Tags.Contains(FName(TEXT("VEIL_BAKE")))) { bAlreadyBuilt = true; break; }
		}
		if (bAlreadyBuilt)
		{
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL STAGE: map=%s already holds the baked district - builder stands down, the saved world is the world."),
				*GetWorld()->GetName());
		}
		else if (bPlacedWorld) { Builder->BuildDistrictLayer(); }
		else { Builder->BuildWorld(); }

		// THE NEST HAS A FLOOR EVEN WHEN THE HUB IS BURIED.
		//
		// On a baked world the builder stands down and the level is the world - and at the
		// plaza centre that level stacks roofs at 500, 1250, 1751 and 2440 cm, so the
		// authority spawn at 0,0,100 lands inside solid geometry and the runner wakes with
		// the camera in a wall. The sanctum measures the surface the hub actually has and
		// raises its deck there, then dresses it: ring, pillars, Spire, beacon shafts and
		// water. It is built before the seat timer below, so the probe lands on the deck.
		if (bAlreadyBuilt)
		{
			// TWO SECONDS LATE, ON PURPOSE. The hub's lid is a streamed cell: at 0.3 s the only
			// collision registered at the plaza is the street, so the deck measured 98 and was
			// raised UNDER the mass it was meant to stand above. Two seconds later it answers.
			FTimerHandle SanctumTimer;
			GetWorldTimerManager().SetTimer(SanctumTimer,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					if (AVEILNestSanctum* Sanctum = GetWorld()->SpawnActor<AVEILNestSanctum>(
						FVector::ZeroVector, FRotator::ZeroRotator))
					{
						Sanctum->BuildSanctum();
					// THE COMPOSITION, on the same tick as the sanctum: pillars, market,
					// falls, bridges, crystals, the Deep, and wings, all placed by phi.
					if (AVEILConceptWorld* Concept = GetWorld()->SpawnActor<AVEILConceptWorld>(
						FVector::ZeroVector, FRotator::ZeroRotator))
					{
						Concept->Build();

					// THE COUNTRY HAS INHABITANTS: six species, their camps, and mines.
					if (AVEILWildsPopulation* Wilds = GetWorld()->SpawnActor<AVEILWildsPopulation>(
						FVector::ZeroVector, FRotator::ZeroRotator))
					{
						Wilds->Populate(14, 24);
					}
					// THE MARKET HAS PEOPLE IN IT: two instanced meshes, one tick.
					if (AVEILCrowd* Crowd = GetWorld()->SpawnActor<AVEILCrowd>(
						FVector::ZeroVector, FRotator::ZeroRotator))
					{
						Crowd->Populate(48);
					}
					}
					}
				}), 2.0f, false);
		}

		// The look is applied after the world exists, whether the level was baked or the
		// builder just raised it. It is computed from the law, so it is the same grade in
		// every session and every build.
		ApplyPhiGrade();

		// DETERMINISTIC PROOF, NOT A KEYSTROKE.
		//
		// The packaged build does not accept injected console input, so a screenshot that
		// has to be typed can never be a check - and a measurement taken off a different
		// camera each time is not an A/B either. With -phishot the game photographs itself
		// 18 seconds in, after the grade has landed: same camera, same moment, same code
		// path, every run. That is what makes the rung comparison mean something.
		if (FParse::Param(FCommandLine::Get(), TEXT("phishot")))
		{
			FTimerHandle ShotTimer;
			GetWorldTimerManager().SetTimer(ShotTimer, [this]()
			{
				if (GEngine)
				{
					// LEVEL THE PROOF CAMERA BEFORE IT FIRES.
					//
					// The seat is aimed down (-17.0324 deg, the golden pitch) at the Keeper's
					// column, and from the plaza centre that put three metres of paving in the
					// frame and the world along its top edge. A proof photo of the floor is not
					// a proof of the world. The shot is taken level, so the buildings, the
					// columns and the skyline are what the camera actually looks at.
					if (APlayerController* ShotPC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
					{
						const FRotator Level(0.f, ShotPC->GetControlRotation().Yaw, 0.f);
						ShotPC->SetControlRotation(Level);
						UE_LOG(LogVEIL, Warning, TEXT("VEIL SHOT LEVEL: frame taken at yaw %.0f, pitch 0 - horizon, not paving."),
							Level.Yaw);
					}
					GEngine->Exec(GetWorld(), TEXT("HighResShot 1280x720"));
					UE_LOG(LogVEIL, Warning, TEXT("VEIL SHOT: photographed from inside the game, no keystroke involved."));
				}
			}, 18.f, false);
		}

		// THE RUNNER WAKES IN OPEN AIR, NOT INSIDE THE HEART OF THE WORLD.
		// PlazaCenter is the world's centre, and the centre is OCCUPIED: the
		// Megastructure's own lamp sphere stands at 0,0,200 with a 90 cm radius, so
		// a body seated at 0,0,220 opened Play inside a glowing ball - a black frame
		// with one lit rim and the lens 13 cm from its inner wall. Seating is
		// therefore PROBED, never assumed: a golden-angle sunflower walks outward
		// from the plaza and the first point with real floor under it AND room to
		// swing the third-person arm wins.
		//
		// IT IS PROBED ONE PHI-SLICE LATE, ON PURPOSE. The street layer was
		// registered in THIS frame; its physics state is not queryable yet, so a
		// probe run here traced a world with no floor, found nothing, and left the
		// body at the centre - which is the orb. Half a second later the geometry
		// answers and the same probe seats him for real.
		{
			TWeakObjectPtr<AVEILGameMode> WeakThis(this);
			const FVector Plaza = Builder->PlazaCenter;
			FTimerHandle SeatTimer;
			GetWorldTimerManager().SetTimer(SeatTimer, [WeakThis, Plaza]()
			{
				AVEILGameMode* Self = WeakThis.Get();
				UWorld* W = Self ? Self->GetWorld() : nullptr;
				APawn* Pawn = UGameplayStatics::GetPlayerPawn(Self, 0);
				if (!W || !Pawn) { return; }

				const float GAngleDeg = 137.507764f;
				const float Roomy = 42.f * 1.618033988749895f * 1.618033988749895f;

				FCollisionQueryParams Params(SCENE_QUERY_STAT(VeilSeatProbe), false);
				Params.AddIgnoredActor(Pawn);

				// THE VAULT IS A LANDMARK, NOT A FLOOR. Its bounds reach z=747 over the
				// plaza centre, so the highest hit under a candidate was its roof and the
				// runner woke on top of the Vault at 598 instead of on the plaza. Ignored
				// here, the same golden-angle walk finds the floor it stands on.
				for (TActorIterator<AActor> VaultIt(W); VaultIt; ++VaultIt)
				{
					if (*VaultIt && VaultIt->GetClass()->GetName().Contains(TEXT("Vault")))
					{
						Params.AddIgnoredActor(*VaultIt);
					}
				}

				// A LANDMARK'S COLLISION PROXY IS NOT A FLOOR EITHER. The runner kept
				// waking at z=598 beside a gold column, and this is the cause: the trace
				// under the plaza does not reach the ground, it hits Actor_1 - a meshless
				// collision slab whose top face is at z=500 and which spans the plaza in
				// every direction - so he was seated 5 m up, on a block, next to
				// SM_AncientColumn_Big. The Vault ignore above cannot catch it because the
				// actor is not named "Vault". A surface with no mesh on it is a proxy, so
				// whatever the centre trace hits that carries no static mesh, above plaza
				// level, is ignored; the same trace then reaches the floor the district
				// is actually built on.
				{
					FHitResult CentreHit;
					FCollisionQueryParams ProbeParams(SCENE_QUERY_STAT(VeilSeatProxy), false);
					ProbeParams.AddIgnoredActor(Pawn);
					if (W->LineTraceSingleByChannel(CentreHit, Plaza + FVector(0.f, 0.f, 4000.f),
						Plaza - FVector(0.f, 0.f, 4000.f), ECC_Visibility, ProbeParams))
					{
						AActor* HitActor = CentreHit.GetActor();
						const bool bIsMesh = Cast<AStaticMeshActor>(HitActor) != nullptr;
						if (HitActor && !bIsMesh && CentreHit.ImpactPoint.Z > Plaza.Z + 150.f)
						{
							Params.AddIgnoredActor(HitActor);
							UE_LOG(LogVEIL, Warning, TEXT("VEIL SEAT: ignored the meshless collision proxy %s (top z=%.0f) - a landmark, not a floor."),
								*HitActor->GetName(), CentreHit.ImpactPoint.Z);
						}
					}
				}

				// AUTHORED SPOTS FIRST. Two rounds of tuning the spiral still produced a
				// bad opening frame - once on a roof, once wedged against a gold column -
				// so the seat no longer depends on a search. These five points are on the
				// plaza approach the region tour already walks, and the first one with
				// real ground, standing room and nine clear metres toward the Keeper is
				// where the runner wakes. The spiral stays underneath as the fallback it
				// always was, for the case where the district changes and none of these
				// five is open any more.
				FVector Seat = Plaza;
				FVector Fallback = FVector::ZeroVector;
				bool bRoom = false;
				int32 MissFloor = 0, Blocked = 0, Roofs = 0;
				TSet<AActor*> RoofsIgnored;
				// THE SEARCH WIDENS BY PHI UNTIL THE WORLD HAS ROOM. At street scale
				// (15 m) the plaza is built over - every candidate had floor but no
				// capsule could stand in it - so the spiral steps out three phi rings,
				// 260 / 620 / 1300 cm per step, up to 36 m. The runner wakes in the
				// first open ground the world actually has, at road scale if needed.
				if (!bRoom)
				{
					const FVector Spots[] = {
						FVector(900.f, -1500.f, Plaza.Z), FVector(-900.f, -1500.f, Plaza.Z),
						FVector(0.f, -1800.f, Plaza.Z), FVector(1400.f, -900.f, Plaza.Z),
						FVector(-1400.f, -900.f, Plaza.Z) };
					for (const FVector& Spot : Spots)
					{
						FHitResult Floor;
						if (!W->LineTraceSingleByChannel(Floor, Spot + FVector(0.f, 0.f, 4000.f),
							Spot - FVector(0.f, 0.f, 4000.f), ECC_Visibility, Params)) { continue; }
						if (Floor.ImpactPoint.Z > Plaza.Z + 40.f) { continue; }
						const FVector Stand(Floor.ImpactPoint.X, Floor.ImpactPoint.Y,
							Floor.ImpactPoint.Z + 98.f);
						FCollisionQueryParams SpotParams = Params;
						SpotParams.AddIgnoredActor(Floor.GetActor());
						if (W->OverlapBlockingTestByChannel(Stand, FQuat::Identity, ECC_Pawn,
							FCollisionShape::MakeCapsule(Roomy, 96.f), SpotParams)) { continue; }
						FHitResult Ahead;
						const FVector Eye = Stand + FVector(0.f, 0.f, 60.f);
						if (W->LineTraceSingleByChannel(Ahead, Eye, Eye + FVector(0.f, -900.f, 0.f),
							ECC_Visibility, SpotParams)) { continue; }
						Seat = Stand;
						bRoom = true;
						UE_LOG(LogVEIL, Log, TEXT("VEIL SEAT: authored spot %s is open - runner wakes at %s."),
							*Spot.ToCompactString(), *Stand.ToCompactString());
						break;
					}
				}

				for (float StepCm : { 260.f, 620.f, 1300.f })
				{
					if (bRoom) { break; }
				for (int32 i = 1; i <= 34; ++i)
				{
					const float Ang = FMath::DegreesToRadians(i * GAngleDeg);
					const float Rad = StepCm * FMath::Sqrt((float)i);
					const FVector Try(Plaza.X + FMath::Cos(Ang) * Rad,
						Plaza.Y + FMath::Sin(Ang) * Rad, Plaza.Z);

					// THE SPIRE'S BASE IS NOT A SEAT. The centre of the plaza is where the
					// stacked gold columns stand - 6.4 m wide at the bottom - and a candidate
					// 620 cm out is close enough that the camera's whole frame is gold. The
					// keep-out is phi^2 steps from it: 1097 cm, wider than the column and its
					// plinth, so no seat can end up looking up the shaft.
					if (FVector::Dist2D(Try, FVector(0.f, 0.f, 0.f)) < 1097.f) { continue; }

					FHitResult Floor;
					if (!W->LineTraceSingleByChannel(Floor, Try + FVector(0.f, 0.f, 4000.f),
						Try - FVector(0.f, 0.f, 4000.f), ECC_Visibility, Params))
					{
						++MissFloor;
						continue;
					}

					// A ROOF IS NOT A FLOOR, AND SKIPPING IT IS NOT ENOUGH. Whatever covers
					// the plaza - a gold plinth at z=500 here - is hit by EVERY candidate, so
					// simply rejecting the candidate fails the whole walk (that attempt sent
					// him into mid-air at z=440). The roof has to be taken out of the query:
					// the actor is added to the ignore list the first time it is seen, and the
					// next trace reaches the ground underneath it.
					// GROUND, NOT A TERRACE. Plaza level is the floor the district is
					// built on; a hit a hundred centimetres above it is a terrace lip, and
					// the seat that accepted one woke him at z=445 - in the air again. A
					// step and a half is the whole tolerance.
					if (Floor.ImpactPoint.Z > Plaza.Z + 40.f)
					{
						if (AActor* RoofActor = Floor.GetActor())
						{
							if (!RoofsIgnored.Contains(RoofActor))
							{
								RoofsIgnored.Add(RoofActor);
								Params.AddIgnoredActor(RoofActor);
								UE_LOG(LogVEIL, Warning, TEXT("VEIL SEAT: %s is a roof at z=%.0f, not a floor - taken out of the seat query."),
									*RoofActor->GetName(), Floor.ImpactPoint.Z);
							}
						}
						++Roofs;
						continue;
					}

					// A BODY STANDING ON THE FLOOR DOES NOT OVERLAP IT. The capsule was
					// seated with its feet exactly ON the floor plane, so the overlap
					// test reported the floor itself as a blocker and every candidate was
					// thrown away - the probe looked like "no floor" when it was really
					// "touching floor". Two centimetres of air, and the floor's own actor
					// ignored: what is left is the question that was meant.
					const FVector Stand(Floor.ImpactPoint.X, Floor.ImpactPoint.Y,
						Floor.ImpactPoint.Z + 98.f);
					FCollisionQueryParams StandParams = Params;
					StandParams.AddIgnoredActor(Floor.GetActor());
					if (W->OverlapBlockingTestByChannel(Stand, FQuat::Identity, ECC_Pawn,
						FCollisionShape::MakeCapsule(42.f, 96.f), StandParams))
					{
						++Blocked;
						continue;
					}
					if (Fallback.IsZero()) { Fallback = Stand; }
					// Room to swing the arm, not merely room to stand.
					if (W->OverlapBlockingTestByChannel(Stand, FQuat::Identity, ECC_Pawn,
						FCollisionShape::MakeCapsule(Roomy, 96.f), StandParams)) { continue; }

					// A CLEAR ARM IS NOT A CLEAR VIEW. The seat passed every capsule test
					// while the opening frame was a wall of gold - the runner stood at the
					// base of SM_AncientColumn_Big, so the camera sat inside the column and
					// the quest line pointed at nothing visible. Standing room is not
					// enough: nine metres ahead at eye height, toward the Keeper the quest
					// names, has to be open, or the walk moves on to the next candidate.
					{
						FHitResult Ahead;
						FCollisionQueryParams ViewParams = StandParams;
						const FVector Eye = Stand + FVector(0.f, 0.f, 60.f);
						if (W->LineTraceSingleByChannel(Ahead, Eye, Eye + FVector(0.f, -900.f, 0.f),
							ECC_Visibility, ViewParams))
						{
							++Blocked;
							continue;
						}
					}

					Seat = Stand;
					bRoom = true;
					UE_LOG(LogVEIL, Log, TEXT("VEIL SEAT: plaza centre is occupied - runner wakes at %s, %.0f cm out on the golden angle (step %.0f, i=%d); %d point(s) had no floor, %d had no room, %d roof(s) struck."),
						*Stand.ToCompactString(), Rad, StepCm, i, MissFloor, Blocked, Roofs);
					break;
				}
				if (bRoom) { break; }
				}
				if (!bRoom && !Fallback.IsZero())
				{
					Seat = Fallback;
					UE_LOG(LogVEIL, Log, TEXT("VEIL SEAT: only a tight fit outside the centre - runner wakes at %s."),
						*Seat.ToCompactString());
				}
				else if (!bRoom)
				{
					// THE HUB PAD, NOT THE SPIRE'S FEET. The fallback used to drop the runner
					// on the plaza centre, which is where the Spire stands: he woke six metres
					// from a two-metre-thick gold column, so every opening frame was gold
					// filling the middle of the shot. The Hub arrival pad is the square the
					// region tour already proves standable, and it looks across the plaza
					// instead of up it.
					FHitResult PadFloor;
					FCollisionQueryParams PadParams = Params;
					const FVector Pad(900.f, -1500.f, 0.f);
					if (W->LineTraceSingleByChannel(PadFloor, Pad + FVector(0.f, 0.f, 4000.f),
						Pad - FVector(0.f, 0.f, 4000.f), ECC_Visibility, PadParams))
					{
						Seat = PadFloor.ImpactPoint + FVector(0.f, 0.f, 98.f);
						bRoom = true;
						UE_LOG(LogVEIL, Warning, TEXT("VEIL SEAT: plaza centre holds nothing but the Spire - runner wakes on the Hub pad at %s, not at its feet."),
							*Seat.ToCompactString());
					}
					else
					{
						Seat = Plaza + FVector(0.f, 0.f, 220.f);
					UE_LOG(LogVEIL, Log, TEXT("VEIL SEAT: nothing at street scale held him - %d point(s) with no floor, %d with no room - runner dropped at %s."),
						MissFloor, Blocked, *Seat.ToCompactString());
					}
				}

				// NOTHING MAY BE STANDING WHERE THE RUNNER WAKES. The search above asks
				// about floor and about room, and it answers both - but the district is
				// dressed by the builder at Play, and a piece of it can occupy the seat
				// after that probe settles. That is the frame the runner woke in: at
				// V(-512,-91,598) with his shoulder inside a gold mass and the camera
				// buried in it. No saved-level or pivot probe can see that mass (it is
				// not in the saved level), but COLLISION can, so the last step is the
				// only one that matters: if the capsule is blocked where he is about to
				// stand, walk him out on the golden angle, and lift him if the mass is
				// tall, until he has air around him. If nothing is clear, he keeps the
				// seat and the log says so instead of pretending.
				{
					// WHAT WAS NOT A FLOOR IS NOT A WALL EITHER.
					//
					// This check used to start a fresh query that ignored only the pawn, so the
					// slab (top 500), the landmark proxies (top 4220) and the terraces the seat
					// search had already dismissed were blocking again with full authority. Every
					// candidate inside 34 golden steps read as blocked, the runner was clamped
					// back to 0,0, and the camera sat inside the mass: "floor Actor_0 at 0 cm".
					// It now inherits the ignored set it is continuing, and adds the body.
					FCollisionQueryParams FreeParams = Params;
					FreeParams.AddIgnoredActor(Pawn);
					auto BlockedAt = [&](const FVector& At)
					{
						return W->OverlapBlockingTestByChannel(At, FQuat::Identity, ECC_Pawn,
							FCollisionShape::MakeCapsule(42.f, 96.f), FreeParams);
					};
					if (BlockedAt(Seat))
					{
						FVector Best = Seat;
						bool bFree = false;
						// A LIFT IS FOR CLEARING A KERB, NOT FOR TAKING OFF. The first version
						// of this offered 520 and 800 cm and the runner, blocked on good ground
						// at z=119, was lifted to z=639 - hovering over the plaza. Steps only.
						for (float Lift : { 0.f, 40.f, 90.f })
						{
							for (int32 i = 1; i <= 34; ++i)
							{
								const float Ang = FMath::DegreesToRadians(i * GAngleDeg);
								const float Rad = 260.f * FMath::Sqrt((float)i);
								const FVector Try(Seat.X + FMath::Cos(Ang) * Rad,
									Seat.Y + FMath::Sin(Ang) * Rad, Seat.Z + Lift);
								if (!BlockedAt(Try)) { Best = Try; bFree = true; break; }
							}
							if (bFree) { break; }
						}
						UE_LOG(LogVEIL, Warning, TEXT("VEIL SEAT: the runner woke inside something at %s - %s."),
							*Seat.ToCompactString(),
							bFree
								? *FString::Printf(TEXT("walked out to %s on the golden angle"), *Best.ToCompactString())
								: TEXT("nothing within 34 golden steps was clear; the seat stands as found"));
						if (bFree) { Seat = Best; }
					}
				}

				// THE RUNNER SPAWNS AT HIS AUTHORITY ADDRESS. The Bible fixes RUNNER at
				// 0,0,100; the search above was allowed to walk him as far as it liked, and it
				// ended 3.2 km away to escape clutter - solving a dressing problem by moving
				// the game. The Nest is cleared now, so the spawn is the plaza centre and the
				// world is fixed to it, not the other way round. A seat that lands outside
				// 8 m of the address is replaced by the address itself, on its own floor.
				// THE CLAMP IS A LEASH, NOT A CAGE. Eight metres was tight enough that the
				// probe's own answer - floor, room, clear view - was discarded and the runner
				// put back inside the hub's base. The plaza ring is the boundary of the
				// address; past it the address wins, inside it the proved seat does.
				const float AddressLeash = 2600.f;
				if (FVector::Dist2D(Seat, FVector::ZeroVector) > AddressLeash)
				{
					// DO NOT TRACE FROM INSIDE THE ROOF. This clamp traced down from z=+4000 and
					// its very first hit was the meshless slab whose top is at 500 - so "the
					// authority spawn" put the runner at z=598.15, five metres up on a landmark,
					// and the camera probe said so: runner V(Z=598.15). Start above everything,
					// skip every hit that is not street level, and take the first one that is.
					FCollisionQueryParams CentreParams = Params;
					FVector From(0.f, 0.f, 60000.f);
					bool bLanded = false;
					for (int32 Step = 0; Step < 12; ++Step)
					{
						FHitResult Centre;
						if (!W->LineTraceSingleByChannel(Centre, From, FVector(0.f, 0.f, -4000.f),
							ECC_Visibility, CentreParams))
						{
							break;
						}
						const float HitZ = Centre.ImpactPoint.Z;
						// STREET LEVEL IS ~0-20 CM, NOT "ANYTHING UNDER 600". The slab's top is at
						// 500, which passed a 600 cm test and put the runner at 598.15 - the same
						// roof, one threshold lower. 120 cm is the plaza floor and its paving, and
						// nothing above it counts as ground.
						if (HitZ < 120.f)
						{
							Seat = FVector(0.f, 0.f, HitZ + 98.f);
							bLanded = true;
							break;
						}
						From = FVector(0.f, 0.f, HitZ - 25.f);
					}
					if (!bLanded) { Seat = FVector(0.f, 0.f, 98.f); }
					UE_LOG(LogVEIL, Warning, TEXT("VEIL SEAT: held to the authority spawn - runner wakes at %s, %.0f cm from 0,0 (street level, not the slab)."),
						*Seat.ToCompactString(), FVector::Dist2D(Seat, FVector::ZeroVector));
				}

				{
					// THE ADDRESS IS X,Y - THE FLOOR IS WHATEVER THE WORLD ACTUALLY HAS.
					// The seat was pinned to the anchor's z (100) while the ground under the
					// hub stands far higher, so the runner was born inside the terrain and the
					// capsule was pushed out sideways: spawn logged (-200,0,92) and the first
					// track a second later found him at (-516,473) having pressed nothing.
					// X and Y are never moved here; only a seat BELOW the ground is lifted.
					FHitResult Ground;
					FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(VEILSeatGround), false, Pawn);
					const FVector From(Seat.X, Seat.Y, Seat.Z + 20000.f);
					const FVector To(Seat.X, Seat.Y, Seat.Z - 20000.f);
					if (Pawn->GetWorld()->LineTraceSingleByChannel(Ground, From, To, ECC_Visibility, GroundParams))
					{
						if (Ground.ImpactPoint.Z > Seat.Z + 50.f)
						{
						// A ROOF IS NOT A FLOOR. This trace starts 200 m above the address, so
						// the first surface it meets over the plaza is whatever is ON TOP of it -
						// a roof at z=2440 in this district - and the block below then lifted the
						// runner onto that roof and logged it as "standing on the floor". He
						// spent the run five metres above the market he was supposed to wake in.
						// Only a surface within a storey of the seat is ground he could be
						// sinking through; anything higher is architecture, and the seat is kept.
						const float Rise = (float)Ground.ImpactPoint.Z - Seat.Z;
						if (Rise > 1200.f)
						{
							UE_LOG(LogVEIL, Warning,
								TEXT("VEIL SEAT GROUND: surface at z=%.0f is %.0f cm overhead - a roof, not a floor; seat kept at z=%.0f (%.0f,%.0f)."),
								Ground.ImpactPoint.Z, Rise, Seat.Z, Seat.X, Seat.Y);
						}
						else
						{
							UE_LOG(LogVEIL, Warning,
								TEXT("VEIL SEAT GROUND: anchor z=%.0f but the floor under the address is z=%.0f - standing on the floor, X,Y untouched (%.0f,%.0f)."),
								Seat.Z, Ground.ImpactPoint.Z, Seat.X, Seat.Y);
							Seat.Z = Ground.ImpactPoint.Z + 98.f;
						}
						}
					}
				}

				Pawn->SetActorLocation(Seat, false, nullptr, ETeleportType::TeleportPhysics);

				// THE FIRST FRAME POINTS AT THE FIRST OBJECTIVE. Waking with the Keeper
				// behind the runner made "Talk to the KEEPER beacon" a line of text with
				// nothing in the shot to walk toward. He is turned to the Keeper's column
				// (0,-2600, the one the builder raised with a beam into the sky), so the
				// beacon is the thing in front of him before he touches a key.
				const FVector KeeperAt(0.f, -2600.f, 0.f);
				if (FVector::Dist2D(Seat, KeeperAt) > 1.f)
				{
					const float FaceYaw = (KeeperAt - Seat).Rotation().Yaw;
					Pawn->SetActorRotation(FRotator(0.f, FaceYaw, 0.f));
					if (AController* C = Pawn->GetController())
					{
						C->SetControlRotation(FRotator(-17.0324f, FaceYaw, 0.f));
					}
				}
			}, 0.5f, false);
		}
		for (int32 i = 0; i < 3; ++i)
		{
			const float Angle = (2.f * PI / 3.f) * i;
			FVector Loc(FMath::Cos(Angle) * 950.f,
				FMath::Sin(Angle) * 950.f, 240.f);

			// THE RING IS NOT EMPTY ANY MORE.
			//
			// On a bare map the 950 cm ring is open ground and a blind seat is fine. In the
			// baked district that ring runs THROUGH the market walls, and a spawn that lands
			// inside geometry is not adjusted - it is refused, silently, and the agent is
			// never born (two of the three were lost that way on VEILDistrictWP). A seat is
			// therefore PROBED for the floor it stands on, and the spawn never refuses.
			{
				FHitResult FloorHit;
				FCollisionQueryParams SeatParams(SCENE_QUERY_STAT(PHIRingSeat), false);
				if (APawn* Runner = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
				{
					SeatParams.AddIgnoredActor(Runner);
				}
				if (GetWorld()->LineTraceSingleByChannel(FloorHit,
					FVector(Loc.X, Loc.Y, 3000.f), FVector(Loc.X, Loc.Y, -1000.f),
					ECC_Visibility, SeatParams))
				{
					Loc.Z = FloorHit.ImpactPoint.Z + 98.f;
				}
			}

			FActorSpawnParameters RingParams;
			RingParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (AVEILFactionAgent* Agent =
				GetWorld()->SpawnActor<AVEILFactionAgent>(
					AVEILFactionAgent::StaticClass(), Loc,
					FRotator(0.f, -Angle * 180.f / PI, 0.f), RingParams))
			{
				Agent->SpawnDefaultController();
				UE_LOG(LogVEIL, Display, TEXT("PHI RING SEAT: agent %d at %s (floor probed, never refused)."),
					i, *Loc.ToCompactString());
			}
			else
			{
				UE_LOG(LogVEIL, Warning, TEXT("PHI RING SEAT: agent %d was refused at %s even with AlwaysSpawn."),
					i, *Loc.ToCompactString());
			}
		}
	}

	// Seed the world systems with the authored starting data.
	if (UVEILDiscoveryManager* Discovery = GetWorld()->GetSubsystem<UVEILDiscoveryManager>())
	{
		for (const FVEILMystery& Mystery : MysteriesToRegister)
		{
			Discovery->RegisterMystery(Mystery);
		}
		Discovery->OnMysterySolved.AddDynamic(this, &AVEILGameMode::HandleMysterySolved);
	}

	if (UVEILFactionManager* Factions = GetWorld()->GetSubsystem<UVEILFactionManager>())
	{
		for (const FVEILTerritory& Territory : TerritoriesToRegister)
		{
			Factions->RegisterTerritory(Territory);
		}
		if (ReinforcementAgentClass)
		{
			Factions->ReinforcementClass = ReinforcementAgentClass;
		}
	}

	// SURFACE PROBE: "the floor is white" is a symptom. This names the mesh and the
	// material the runner is actually standing on and looking at, so the look is
	// fixed where it is authored instead of compensated for in the grade.
	if (FParse::Param(FCommandLine::Get(), TEXT("phisurface")))
	{
		FTimerHandle SurfaceTimer;
		GetWorldTimerManager().SetTimer(SurfaceTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			APawn* P = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
			if (!P) { return; }
			auto Report = [](UWorld* W, const FVector& From, const FVector& To, const FString& Label)
			{
				FHitResult Hit;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(PHISurface), false);
				if (!W->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, Params)) return;
				const AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Hit.GetActor());
				const UStaticMeshComponent* C = SMA ? SMA->GetStaticMeshComponent() : nullptr;
				const UStaticMesh* SM = C ? C->GetStaticMesh() : nullptr;
				const UMaterialInterface* Mat = C ? C->GetMaterial(0) : nullptr;
				UE_LOG(LogTemp, Display, TEXT("PHI SURFACE %s: actor=%s mesh=%s mat=%s at %s"),
					*Label, Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("none"),
					SM ? *SM->GetName() : TEXT("none"),
					Mat ? *Mat->GetName() : TEXT("none"),
					*Hit.ImpactPoint.ToCompactString());
			};
			// Every volume that can own the frame, named: three exposure settings in a
			// row changed NO pixel, which means the grade is not the one being blended.
			for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
			{
				UE_LOG(LogTemp, Display,
					TEXT("PHI PPV: %s priority=%.0f unbound=%d weight=%.2f min=%.2f max=%.2f bias=%.2f sat=%.2f"),
					*It->GetName(), It->Priority, It->bUnbound ? 1 : 0, It->BlendWeight,
					It->Settings.AutoExposureMinBrightness, It->Settings.AutoExposureMaxBrightness,
					It->Settings.AutoExposureBias, It->Settings.ColorSaturation.X);
			}
			const FVector P0 = P->GetActorLocation();
			// A RING, not one point: the plaza is a patchwork and the white sheet and the
			// cyan sheets are different actors. Naming every surface under the runner is
			// what turns "the floor looks wrong" into a list of things to fix.
			const float Ring[7][2] = { {0.f, 0.f}, {600.f, 0.f}, {-600.f, 0.f}, {0.f, 600.f}, {0.f, -600.f}, {1200.f, 400.f}, {-1200.f, -400.f} };
			for (int32 i = 0; i < 7; ++i)
			{
				const FVector At(P0.X + Ring[i][0], P0.Y + Ring[i][1], P0.Z + 300.f);
				Report(GetWorld(), At, At - FVector(0.f, 0.f, 900.f),
					*FString::Printf(TEXT("@%d,%d"), (int32)Ring[i][0], (int32)Ring[i][1]));
			}
		}), 4.5f, false);
	}

	// DEV MESSAGES OFF: "LIGHTING NEEDS TO BE REBUILT", Lumen atlas and shadow
	// pool warnings are engine diagnostics for a developer, not for the player.
	// They sat across the middle of Play; the game is not a console log.
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), TEXT("DisableAllScreenMessages"));
	}

	// SCENERY PASS: the district ships world-spanning backdrop meshes (sky shells,
	// nebula props) whose collision encloses the plaza. The runner stood there
	// with acceleration 2048 and velocity 0 - every step blocked by a skybox.
	// Anything the size of a skybox is scenery, not floor: collision off.
	{
		int32 Released = 0;
		for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
		{
			FVector Origin, Extent;
			It->GetActorBounds(false, Origin, Extent);
			if (Extent.GetMax() < 20000.f) continue; // only the world-sized ones
			UStaticMeshComponent* M = It->GetStaticMeshComponent();
			if (!M || M->GetCollisionEnabled() == ECollisionEnabled::NoCollision) continue;
			M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			++Released;
			// The mesh name is the whole point: "a backdrop is in the way" is not
			// something a fix can act on, and a world-sized flat mesh over the plaza
			// paints the district white no matter what the grade says.
			const UStaticMesh* SM = M->GetStaticMesh();
			const UMaterialInterface* Mat = M->GetMaterial(0);
			UE_LOG(LogVEIL, Log,
				TEXT("PHI SCENERY: %s mesh=%s mat=%s origin=%s extent=%s (max %.0f) - collision off"),
				*It->GetName(), SM ? *SM->GetName() : TEXT("none"),
				Mat ? *Mat->GetName() : TEXT("none"),
				*Origin.ToCompactString(), *Extent.ToCompactString(), Extent.GetMax());
		}
		UE_LOG(LogVEIL, Log, TEXT("PHI SCENERY: %d world-spanning mesh(es) released; the district is walkable."), Released);
	}

	// PHI GROUND SKIN: the runner's floor in this level is a giant engine CUBE wearing
	// MI_Iceland_Mountains_01 - a bright mountain material on a 160 m slab. Probed at
	// the runner's feet, named in the log, and it is the white sheet in every frame:
	// no grade can colour a surface that is already clipping.
	//
	// The fix is authored, not compensated: the project's own Sicka stone floor
	// material is put on the placeholder slabs (flat, world-sized, engine primitives
	// only - mountains, cliffs and the sky shell are untouched).
	{
		int32 Skinned = 0;
		// The kit's FLOOR material carries emissive rune inlays that, stretched over a
		// 160 m slab, read as neon cyan sheets - and its emissive is baked into the
		// texture, so no parameter can damp it. The CLIFF material is the same pack's
		// plain cut stone: no glow, real grain, and it takes the sun properly.
		UStaticMesh* StoneAsset = LoadObject<UStaticMesh>(nullptr,
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffBig.SM_CliffBig"));
		UMaterialInterface* Stone = StoneAsset ? StoneAsset->GetMaterial(0) : nullptr;
		if (Stone)
		{
			for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
			{
				UStaticMeshComponent* Comp = It->GetStaticMeshComponent();
				if (!Comp) continue;
				const UStaticMesh* SM = Comp->GetStaticMesh();
				if (!SM) continue;
				const FString MeshName = SM->GetName();
				// Engine primitives only: authored meshes keep their own material.
				if (!(MeshName == TEXT("Cube") || MeshName == TEXT("Plane") || MeshName == TEXT("Cylinder"))) continue;
				FVector Origin, Extent;
				It->GetActorBounds(false, Origin, Extent);
				if (Extent.GetMax() < 2000.f) continue;              // the slab, not a prop
				if (Extent.Z > Extent.X * 0.35f) continue;           // flat surfaces only

				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Stone, Comp);
				if (!MID) continue;
				// Stone, not paint: a dark warm grey that keeps the gold light gold.
				const FLinearColor Ground(0.46f, 0.42f, 0.36f);
				MID->SetVectorParameterValue(TEXT("Color"), Ground);
				MID->SetVectorParameterValue(TEXT("BaseColor"), Ground);
				// Matte stone. The authored floor is polished, and a polished 160 m slab
				// mirrors the blue sky into cyan sheets across the plaza. These scalar
				// writes are inert on a material that does not expose the parameter.
				MID->SetScalarParameterValue(TEXT("Roughness"), 0.92f);
				MID->SetScalarParameterValue(TEXT("Metallic"), 0.f);
				MID->SetScalarParameterValue(TEXT("Specular"), 0.12f);
				// The ancient kit's rune inlays are authored emissive and read as neon
				// cyan sheets across 160 m. Kept, but damped: runes should glow, not glare.
				MID->SetScalarParameterValue(TEXT("EmissiveIntensity"), 0.30f);
				MID->SetScalarParameterValue(TEXT("EmissiveStrength"), 0.30f);
				MID->SetScalarParameterValue(TEXT("Emissive"), 0.30f);
				Comp->SetMaterial(0, MID);
				++Skinned;
				UE_LOG(LogTemp, Display, TEXT("PHI GROUND SKIN: %s (mesh=%s extent %.0f) -> authored stone."),
					*It->GetName(), *MeshName, Extent.GetMax());
			}
		}
		UE_LOG(LogTemp, Display, TEXT("PHI GROUND SKIN: %d placeholder slab(s) re-skinned with the district stone."), Skinned);
	}

	// LIGHT WITHOUT A BAKE: the district's lights are static and its lighting data
	// is stale ("LIGHTING NEEDS TO BE REBUILT"), so Play rendered black. Sun and
	// sky are made movable and re-captured here, so the world is lit the moment
	// the runner lands - no build-lighting pass, no cook, no package.
	{
		int32 Suns = 0, Skies = 0;
		for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
		{
			++Suns;
			It->SetMobility(EComponentMobility::Movable);
			if (UDirectionalLightComponent* Sun = Cast<UDirectionalLightComponent>(It->GetLightComponent()))
			{
				Sun->SetMobility(EComponentMobility::Movable);
				if (Sun->Intensity < 1.f) Sun->SetIntensity(3.14f); // daylight, phi-pitched
				// GOLD LIGHT, BLUE SHADE. A near-white sun over a warm district is what
				// made every surface the same cream: with the sun gold and the sky light
				// cool, the two sides of every pillar separate into two colours.
				Sun->SetLightColor(FLinearColor(1.f, 0.80f, 0.56f));
				Sun->MarkRenderStateDirty();
			}
		}
		for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
		{
			++Skies;
			if (USkyLightComponent* Sky = It->GetLightComponent())
			{
				Sky->SetMobility(EComponentMobility::Movable);
				// 1.45 of cool sky: the fill that keeps the shadow side of every wall blue
				// instead of black, and the reason the gold keeps its colour against it.
				Sky->SetIntensity(FMath::Max(1.45f, Sky->Intensity));
				Sky->SetLightColor(FLinearColor(0.60f, 0.72f, 1.f));
				Sky->RecaptureSky();
			}
		}
		if (Suns == 0)
		{
			if (ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(
				FVector(0.f, 0.f, 2000.f), FRotator(-45.f, UPHIMito::GoldenAngleDeg, 0.f)))
			{
				if (UDirectionalLightComponent* C = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
				{
					C->SetMobility(EComponentMobility::Movable);
					C->SetIntensity(3.14f);
				}
				UE_LOG(LogVEIL, Log, TEXT("PHI LIGHT: no sun in the level - a movable one now stands at the golden angle."));
			}
		}
		if (Skies == 0)
		{
			if (ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>(FVector(0.f, 0.f, 4000.f), FRotator::ZeroRotator))
			{
				if (USkyLightComponent* C = Sky->GetLightComponent())
				{
					C->SetMobility(EComponentMobility::Movable);
					C->SetIntensity(1.f);
					C->RecaptureSky();
				}
				UE_LOG(LogVEIL, Log, TEXT("PHI LIGHT: sky light raised and captured for play."));
			}
		}
		// AIR: a district with no atmosphere reads as a level, not a place. Fog
		// is coded here - no Fab sky pack, no download - and lifts with height so
		// the towers still stand clear of it.
		int32 Fogs = 0;
		for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It)
		{
			++Fogs;
			if (UExponentialHeightFogComponent* F = It->GetComponent())
			{
				F->SetMobility(EComponentMobility::Movable);
				// 0.012 of a violet-grey fog over every surface IS the milky veil in the
				// frame: it was adding ~50% of its own colour to the whole image. A fifth
				// of the density, hugging the ground instead of the towers, keeps the
				// depth and gives the colours back.
				F->SetFogDensity(0.005f);
				F->SetFogHeightFalloff(0.45f);
				F->SetFogInscatteringColor(FLinearColor(0.20f, 0.26f, 0.40f));
				F->SetVolumetricFog(false);
			}
		}
		if (Fogs == 0)
		{
			if (AExponentialHeightFog* Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(
				FVector(0.f, 0.f, 0.f), FRotator::ZeroRotator))
			{
				if (UExponentialHeightFogComponent* F = Fog->GetComponent())
				{
					F->SetMobility(EComponentMobility::Movable);
					F->SetFogDensity(0.005f);
					F->SetFogHeightFalloff(0.45f);
					F->SetFogInscatteringColor(FLinearColor(0.20f, 0.26f, 0.40f));
					F->SetVolumetricFog(false);
				}
				UE_LOG(LogVEIL, Log, TEXT("PHI AIR: height fog raised - the district has depth now."));
			}
		}
		UE_LOG(LogVEIL, Log, TEXT("PHI LIGHT: suns=%d skies=%d fogs=%d made movable - the district is lit without a bake."), Suns, Skies, Fogs);
	}

	// LIVE MACHINES AROUND THE RUNNER: six of them, golden angle apart so no two
	// share a lane, rings stepping by phi for depth. Each seat is traced to the
	// floor and always spawns - the sixth used to be dropped on a collision.
	{
		FVector Centre = FVector::ZeroVector;
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			Centre = Pawn->GetActorLocation();
		}
		int32 Seated = 0;
		FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(PhiMachineSeat), false, this);
		for (int32 i = 0; i < 6; ++i)
		{
			const float AngleDeg = i * UPHIMito::GoldenAngleDeg;
			const float Angle = FMath::DegreesToRadians(AngleDeg);
			const float Radius = 950.f * UPHIMito::PhiPow(static_cast<float>(i % 3));
			const FVector Wish = Centre + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 240.f);

			FVector Seat = Wish;
			FHitResult Ground;
			if (GetWorld()->LineTraceSingleByChannel(Ground, Wish + FVector(0.f, 0.f, 400.f),
				Wish - FVector(0.f, 0.f, 2500.f), ECC_Visibility, GroundParams))
			{
				Seat.Z = Ground.ImpactPoint.Z + 96.f; // capsule half-height, on the floor
			}
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AVEILFactionAgent* Agent = GetWorld()->SpawnActor<AVEILFactionAgent>(
				AVEILFactionAgent::StaticClass(), Seat, FRotator(0.f, AngleDeg, 0.f), SpawnParams))
			{
				Agent->SpawnDefaultController();
				Agent->OnAgentDown.AddDynamic(this, &AVEILGameMode::HandleCampMachineDown);
				++Seated;
			}
		}
		CampStanding = Seated;
		UE_LOG(LogVEIL, Log, TEXT("PHI MACHINES: %d machine(s) seated around the runner at %.0f,%.0f on the golden angle."),
			Seated, Centre.X, Centre.Y);
		// The Vault is told how many hold the plaza before anyone can ask it to open.
		for (TActorIterator<AVEILVault> It(GetWorld()); It; ++It)
		{
			It->CampStanding = CampStanding;
			It->bCampCleared = (CampStanding == 0);
		}
		UE_LOG(LogVEIL, Log, TEXT("PHI CAMP: %d machine(s) hold the district; the Vault stays sealed until they fall."),
			CampStanding);

		// PHI CAMERA PROBE: four seconds in, say where the lens is, how far it is
		// from the runner, and the first thing it is looking at. The opening view
		// stops being a guess and becomes a measurement.
		if (UWorld* ProbeWorld = GetWorld())
		{
			FTimerHandle CameraProbeTimer;
			ProbeWorld->GetTimerManager().SetTimer(CameraProbeTimer,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					APawn* Runner = UGameplayStatics::GetPlayerPawn(this, 0);
					if (!Runner) return;
					UCameraComponent* Lens = Runner->FindComponentByClass<UCameraComponent>();
					if (!Lens) return;
					const FVector LensLocation = Lens->GetComponentLocation();
					const FVector LensForward = Lens->GetForwardVector();
					UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA PROBE: runner %s lens %s fwd %s, %.0f cm from the body."),
						*Runner->GetActorLocation().ToCompactString(), *LensLocation.ToCompactString(),
						*LensForward.ToCompactString(),
						FVector::Dist(LensLocation, Runner->GetActorLocation()));
					FHitResult LensHit;
					FCollisionQueryParams ProbeParams(SCENE_QUERY_STAT(PhiCameraProbe), false, Runner);
					if (GetWorld()->LineTraceSingleByChannel(LensHit, LensLocation,
						LensLocation + LensForward * 2000.f, ECC_Visibility, ProbeParams) && LensHit.GetActor())
					{
						UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA PROBE: first thing in the lens: %s (%s) at %.0f cm."),
							*LensHit.GetActor()->GetName(), *LensHit.GetActor()->GetClass()->GetName(),
							LensHit.Distance);
					}
					else
					{
						UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA PROBE: nothing within 2000 cm of the lens."));
					}
					// GOLD COLUMN COLLISION, MEASURED. The doc says SM_AncientColumn_Big is
					// visual-only and every probe reports clear while the lens sits inside solid
					// gold. Rather than argue, this finds the nearest column and traces straight
					// at it: a hit proves collision exists, "clear" proves it does not.
					{
						AStaticMeshActor* NearestColumn = nullptr;
						float BestDist = 1.0e9f;
						for (TActorIterator<AStaticMeshActor> ColIt(GetWorld()); ColIt; ++ColIt)
						{
							UStaticMeshComponent* MC = ColIt->GetStaticMeshComponent();
							if (!MC || !MC->GetStaticMesh()) { continue; }
							if (!MC->GetStaticMesh()->GetName().Contains(TEXT("AncientColumn"))) { continue; }
							const float D = FVector::Dist(ColIt->GetActorLocation(), Runner->GetActorLocation());
							if (D < BestDist) { BestDist = D; NearestColumn = *ColIt; }
						}
						if (NearestColumn)
						{
							FHitResult ColHit;
							FCollisionQueryParams ColParams(SCENE_QUERY_STAT(PhiColumnProbe), false, Runner);
							// IGNORE THE MESHLESS PROXIES. Actor_1 is a district-wide invisible body whose
							// top is at z=500; any horizontal trace near the ground starts INSIDE it and
							// returns a 0 cm hit, which masks every column behind it. Every actor with no
							// static mesh is ignored, so the trace can only report a real column body.
							for (TActorIterator<AActor> IgnoreIt(GetWorld()); IgnoreIt; ++IgnoreIt)
							{
								AActor* Candidate = *IgnoreIt;
								if (!Candidate || Candidate == NearestColumn || Candidate == Runner) { continue; }
								if (!Candidate->FindComponentByClass<UStaticMeshComponent>())
								{
									ColParams.AddIgnoredActor(Candidate);
								}
							}
							// TRACE ABOVE THE SLAB. The first version aimed from the lens at the column's
							// base and hit Actor_1 at 579 cm - the meshless collision proxy whose top is
							// at z=500 - so it measured the proxy, not the column. This traces
							// horizontally at z = +700, which is above that proxy and inside the column's
							// own body, from 800 cm out straight at its centre.
							// AIM AT THE COLUMN'S REAL BODY, NOT A GUESSED HEIGHT. Aiming at origin + 700
							// cleared every time because that is above the top of a 400-600 cm column:
							// the trace sailed over the thing it was meant to test. The bounds centre is
							// where the geometry actually is.
							const FBox ColBox = NearestColumn->GetComponentsBoundingBox(true);
							const FVector Target = ColBox.IsValid ? ColBox.GetCenter() : NearestColumn->GetActorLocation();
							FVector Out = (Runner->GetActorLocation() - ColBox.GetCenter()).GetSafeNormal2D();
							if (Out.IsNearlyZero()) { Out = FVector(1.f, 0.f, 0.f); }
							const FVector Start = Target + Out * 800.f;
							const bool bColHit = GetWorld()->LineTraceSingleByChannel(ColHit,
								Start, Target, ECC_Visibility, ColParams);
							FString Detail = TEXT("CLEAR - no collision on the column");
							if (bColHit && ColHit.GetActor())
							{
								Detail = FString::Printf(TEXT("HIT %s at %.0f cm"),
									*ColHit.GetActor()->GetName(), ColHit.Distance);
							}
							UE_LOG(LogVEIL, Log, TEXT("PHI COLUMN PROBE: nearest column %s is %.0f cm away; trace into its bounds centre %s (box z %.0f..%.0f)"),
								*NearestColumn->GetName(), BestDist, *Detail,
								ColBox.IsValid ? ColBox.Min.Z : 0.f, ColBox.IsValid ? ColBox.Max.Z : 0.f);
						}
					}
				}), 4.f, false);
		}

		// WHO ENCASES THE RUNNER: the runner stands but cannot walk, so something
		// large is enclosing his capsule. Name every actor whose bounds hold him -
		// one log line instead of a dozen rebuilds.
		if (APawn* Runner = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			const FVector At = Runner->GetActorLocation();
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				AActor* A = *It;
				if (!A || A == Runner || A->IsA(AVEILFactionAgent::StaticClass())) continue;
				// AVolume is not world geometry. The nav bounds volume encloses the
				// runner by design, so naming it here was a guaranteed false positive
				// sitting on top of the real answer the probe exists to find.
				if (A->IsA(AVolume::StaticClass())) continue;
				FVector Origin, Extent;
				A->GetActorBounds(false, Origin, Extent);
				if (Extent.GetMax() < 200.f) continue; // only prop-sized geometry
				// A skybox, a backdrop dome or a kilometre-wide terrain shell holds
				// everything by construction - reporting it buries the one answer this
				// probe exists to give. Prop scale is the whole search space: a capsule
				// is 96 cm half-height, so nothing legitimately enclosing it is bigger
				// than phi^5 (11.09) times a metre of reach.
				if (Extent.GetMax() > 1109.f) continue;
				if (FMath::Abs(At.X - Origin.X) < Extent.X && FMath::Abs(At.Y - Origin.Y) < Extent.Y
					&& At.Z > Origin.Z - Extent.Z && At.Z < Origin.Z + Extent.Z)
				{
					UE_LOG(LogVEIL, Warning, TEXT("PHI ENCASE: %s (%s) holds the runner: origin %s extent %.0f"),
						*A->GetName(), *A->GetClass()->GetName(), *Origin.ToCompactString(), Extent.GetMax());
				}
			}
		}
	}

	SetPhase(EVEILGamePhase::Exploration);
	UE_LOG(LogVEIL, Log, TEXT("VEIL world booted. Phase: Exploration. Difficulty x%.2f"), DifficultyScale);
	FPHIVoice::Play(GetWorld(), TEXT("V01"));

	// PROOF SWITCHES: -phikill answers the nearest machine through the real damage
	// chain so a kill and its XP are proven in a live frame.
	bPhiWinProof = FParse::Param(FCommandLine::Get(), TEXT("phiwin"));
	if (FParse::Param(FCommandLine::Get(), TEXT("phikill")))
	{
		GetWorldTimerManager().SetTimer(PhiKillTimer, this, &AVEILGameMode::PhiKillStep, 4.f, true, 3.f);
		UE_LOG(LogVEIL, Log, TEXT("PHI KILL PROOF: armed - the nearest machine is answered through ApplyDamage."));
	}
}

// ---------------------------------------------------------------------------
// The runner is always spawned
// ---------------------------------------------------------------------------
APawn* AVEILGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
	const FTransform& SpawnTransform)
{
	UWorld* W = GetWorld();
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!W || !PawnClass) return nullptr;

	// The capsule is read from the pawn's own class: no hand-typed size.
	float CapsuleRadius = 34.f;
	float CapsuleHalf = 88.f;
	if (const ACharacter* Default = Cast<ACharacter>(PawnClass->GetDefaultObject()))
	{
		if (const UCapsuleComponent* Caps = Default->GetCapsuleComponent())
		{
			CapsuleRadius = Caps->GetUnscaledCapsuleRadius();
			CapsuleHalf = Caps->GetUnscaledCapsuleHalfHeight();
		}
	}

	const FCollisionShape Shape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalf);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PhiRunnerSeat), false, NewPlayer);

	// THE VAULT IS A LANDMARK, NOT A FLOOR. It stands on the plaza centre with
	// bounds reaching z=747, so the first thing a sky-down seat trace met was its
	// roof: the runner woke on top of the Vault at 598, 260 cm off the seat and
	// unable to walk to the Keeper. Ignored here, the seat is the plaza floor the
	// Vault stands on. The mast on the centre is answered by the same search that
	// already steps off the centre.
	for (TActorIterator<AActor> VaultIt(W); VaultIt; ++VaultIt)
	{
		if (*VaultIt && VaultIt->GetClass()->GetName().Contains(TEXT("Vault")))
		{
			Params.AddIgnoredActor(*VaultIt);
		}
	}

	// The shell: this map carries a world-spanning collision volume (the sky
	// volume at z 160). It answers every probe, so it is named here and ignored
	// in the clearance test below - every other solid stays in play.
	TArray<AActor*> Shells;
	for (TActorIterator<AActor> It(W); It; ++It)
	{
		if (!*It) continue;
		FVector ShellOrigin, ShellExtent;
		It->GetActorBounds(false, ShellOrigin, ShellExtent);
		if (ShellExtent.GetMax() > 5000.f) Shells.Add(*It);
	}

	// A floor is a surface at the runner's own level, not a rooftop: trace from
	// just above the candidate and require a near-vertical, nearby hit.
	auto SeatAt = [&](const FVector& Column, FVector& OutSeat) -> bool
	{
		FHitResult Ground;
		// Start BELOW the sky shell: this map's world-spanning collision volume sits
		// at z 160 and answers every trace first, which made "the floor" 160 cm
		// everywhere and dropped the runner off the world.
		if (!W->LineTraceSingleByChannel(Ground, Column + FVector(0.f, 0.f, 40.f),
			Column - FVector(0.f, 0.f, 1200.f), ECC_Visibility, Params))
		{
			return false; // no floor here: not a seat, however empty it looks
		}
		if (Ground.ImpactNormal.Z < 0.7f) return false; // a wall, not a floor
		// The plaza floor sits ABOVE the runner's anchored z, so the ceiling test
		// has to clear the standing surface it is meant to find, not the anchor.
		if (Ground.ImpactPoint.Z > Column.Z + 200.f) return false; // overhead structure
		OutSeat = Column;
		OutSeat.Z = Ground.ImpactPoint.Z + CapsuleHalf + 2.f;     // capsule bottom ON the floor
		// Clearance: a floor is not a seat if the body standing on it is inside
		// rock. The capsule is tested against everything solid EXCEPT the sky
		// shell named above, so this can pass in the district while still
		// refusing a seat buried in a mountain.
		FCollisionQueryParams ClearParams(SCENE_QUERY_STAT(PhiRunnerClear), false, NewPlayer);
		for (AActor* Shell : Shells)
		{
			ClearParams.AddIgnoredActor(Shell);
		}
		// Visibility, not WorldStatic: rock, props and cliff dressing in this map do
		// not all sit on the static channel, and a seat inside a mountain passed a
		// WorldStatic probe. Everything that blocks sight blocks a body.
		if (W->OverlapBlockingTestByChannel(OutSeat, FQuat::Identity, ECC_Visibility, Shape, ClearParams))
		{
			return false;
		}
		return true;
	};

	// THE RUNNER'S OWN SEAT IS THE ANCHOR: 0,0,100, Tyree's hand-anchored spot.
	const FVector Anchor(VEILAnchors::RunnerX, VEILAnchors::RunnerY, VEILAnchors::RunnerZ);
	const FVector Wish = SpawnTransform.GetLocation();

	FVector Seat = Wish;
	bool bStartSpotHonest = false;
	bool bSeated = false;
	int32 SeatStep = 0;
	float BestOpen = -1.f;

	// Start one step OFF the anchor: the placed RUNNER is the statue standing for
	// him, and a pawn seated in its capsule is pinned - alive, visible, unable to
	// take a step. Every golden-angle step that has clear ground is proved, then
	// SCORED BY OPENNESS: a seat is not good because a body fits in it, it is
	// good because the runner can see where he is going. The plaza's carved wall
	// ring is close enough that a clear capsule can still open onto stone.
	// Step 1 is Tyree's anchored step, and it stays. The inner spiral lands on the
	// plaza's own carved centrepiece, which is decoration WITH NO COLLISION - every
	// shape probe passes through it, so the seat reads clear while the body stands
	// inside stone and the lens fills with its wall. The outer turns are no better:
	// steps 3-8 have no clear ground and step 9 stands in the mountains, which
	// proves the plaza's open ring is narrower than the spiral. The real fix is on
	// the level, not the seat: give the centrepiece collision, or place the
	// Runner's start in a clear plaza cell by hand.
	for (int32 Step = 1; Step < 11; ++Step)
	{
		const float Angle = FMath::DegreesToRadians(UPHIMito::GoldenAngleDeg * static_cast<float>(Step));
		const float Radius = 320.f * UPHIMito::PhiPow(static_cast<float>(Step) * 0.5f);
		const FVector Column = Anchor + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
		FVector Candidate;
		if (!SeatAt(Column, Candidate))
		{
			continue;
		}

		// Openness: the NARROWEST sight line in eight directions from the runner's
		// eye. A single forward trace cannot tell a plaza from a monument, because
		// this map's big carved pieces are decoration with no collision at all - a
		// body can stand inside one and every shape probe still says clear. Eight
		// rays answer the only question that matters: is there room around him.
		const FVector Eye = Candidate + FVector(0.f, 0.f, 60.f);
		FCollisionQueryParams LookParams(SCENE_QUERY_STAT(PhiRunnerView), false, NewPlayer);
		for (AActor* Shell : Shells)
		{
			LookParams.AddIgnoredActor(Shell);
		}
		float Narrowest = 3000.f;
		for (int32 Dir = 0; Dir < 8; ++Dir)
		{
			const float DirAngle = FMath::DegreesToRadians(45.f * static_cast<float>(Dir));
			const FVector Ray(FMath::Cos(DirAngle), FMath::Sin(DirAngle), 0.f);
			FHitResult Look;
			float Open = 3000.f;
			if (W->LineTraceSingleByChannel(Look, Eye, Eye + Ray * Open, ECC_Visibility, LookParams))
			{
				Open = FVector::Dist(Eye, Look.ImpactPoint);
			}
			Narrowest = FMath::Min(Narrowest, Open);
		}
		if (Narrowest > BestOpen)
		{
			BestOpen = Narrowest;
			Seat = Candidate;
			SeatStep = Step;
			bSeated = true;
		}
		// The nearest step with real room ahead is the honest seat: no wandering
		// to the far side of the district for a nicer view.
		if (BestOpen > 1500.f)
		{
			break;
		}
	}

	// THE ANCHOR SETS THE HEIGHT, not whatever slope the floor trace happened to
	// find: the seat was landing 138 cm above Tyree's 0,0,100 and standing the
	// runner on a mountain flank. His z is the anchor's z, and the capsule is
	// never dropped below it.
	if (bSeated)
	{
		Seat.Z = FMath::Clamp(Seat.Z, Anchor.Z, Anchor.Z + 100.f);

		// THE ADDRESS IS X,Y - THE FLOOR IS WHAT THE WORLD HAS THERE.
		// The seat was pinned to the anchor's z, but the ground under the hub stands
		// far above it, so the runner was born inside the terrain and the capsule was
		// pushed out sideways: spawn logged (-200,0,92) and a second later he was at
		// (-516,473) having pressed nothing. X and Y are never moved; a seat that is
		// BELOW the floor is lifted onto it and the discrepancy is named in the log.
		FHitResult RunnerGround;
		FCollisionQueryParams RunnerSP(SCENE_QUERY_STAT(VEILRunnerGround), false);
		const FVector GroundFrom(Seat.X, Seat.Y, Seat.Z + 20000.f);
		const FVector GroundTo(Seat.X, Seat.Y, Seat.Z - 40000.f);
		if (W->LineTraceSingleByChannel(RunnerGround, GroundFrom, GroundTo, ECC_Visibility, RunnerSP))
		{
			if (RunnerGround.ImpactPoint.Z > Seat.Z + 50.f)
			{
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI RUNNER GROUND: anchor z=%.0f but the floor under the address is z=%.0f - standing on the floor, X,Y untouched (%.0f,%.0f)."),
					Seat.Z, RunnerGround.ImpactPoint.Z, Seat.X, Seat.Y);
				Seat.Z = RunnerGround.ImpactPoint.Z + 98.f;
			}
		}
	}

	// ON THE PLAZA FLOOR, AT HIS OWN COORDINATE. PHI GROUND lays nine kit tiles with the
	// top face at z=0 and RUNNER is 0,0,100 - 98 cm of stand-off over the market. The
	// golden-angle step off the anchor seeded him onto a shelf at 578 every run, which is
	// the floating frame: a man standing five metres above his own district.
	Seat = FVector(0.f, -1500.f, 100.f);
		FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient; // never save the player pawn into a map
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTransform Where = SpawnTransform;
	Where.SetLocation(Seat);
	// The camera rides 420 cm BEHIND whatever the runner faces, so the seat alone
	// never decides the opening shot: face the plaza and the Vault's stele fills
	// the lens at point-blank, face outward and the camera sits inside the stele
	// itself. Left as it spawns - the seat is Tyree's anchored step and the facing
	// is his call. What is proven here is the ground (clearance) and the body.
	APawn* Runner = W->SpawnActor<APawn>(PawnClass, Where, SpawnInfo);
	UE_LOG(LogVEIL, Log, TEXT("PHI RUNNER SPAWN: %s at %s (golden-angle step %d off the RUNNER's own seat %s; open ground ahead %.0f cm; %d shell(s) ignored)."),
		*PawnClass->GetName(), *Seat.ToCompactString(),
		SeatStep, *Anchor.ToCompactString(), FMath::Max(BestOpen, 0.f), Shells.Num());
	return Runner;
}

APawn* AVEILGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// THE MAP'S PLAYERSTART DOES NOT DECIDE WHERE THE RUNNER BEGINS. The StartSpot
	// transform was handed straight through, so the authored seat V(0,0,100) was lost
	// and the capsule began at V(-516,472,198) - inside geometry, above the plaza,
	// which is the frame that never moves. RUNNER's own coordinate wins.
	const FTransform Authority(FRotator(0.f, 0.f, 0.f), FVector(0.f, -1500.f, 100.f));
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI SEAT LAW: runner spawns on V(0,0,100) over PHI GROUND, not on %s."),
		StartSpot ? *StartSpot->GetName() : TEXT("a default start spot"));
	return SpawnDefaultPawnAtTransform_Implementation(NewPlayer, Authority);
}

void AVEILGameMode::HandleCampMachineDown(AVEILFactionAgent* Agent)
{
	CampStanding = FMath::Max(0, CampStanding - 1);

	UWorld* W = GetWorld();
	if (!W) return;

	const bool bCleared = (CampStanding == 0);
	for (TActorIterator<AVEILVault> It(W); It; ++It)
	{
		It->CampStanding = CampStanding;
		It->bCampCleared = bCleared;
	}

	UE_LOG(LogVEIL, Log, TEXT("PHI CAMP: %s down - %d machine(s) still hold the district."),
		Agent ? *Agent->GetActorNameOrLabel() : TEXT("a machine"), CampStanding);

	if (bCleared)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI CAMP CLEARED: the plaza is the runner's - the Vault's machine gate is open."));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor(255, 216, 110),
				TEXT("CAMp CLEARED  -  the Vault answers now"));
		}
	}
}

void AVEILGameMode::PhiKillStep()
{
	UWorld* W = GetWorld();
	if (!W) { return; }
	APawn* Runner = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Runner) { return; }

	// Answer the camp one machine at a time - every step works through the real
	// damage chain, and the last one is what opens the Vault's machine gate.
	{
		AVEILFactionAgent* Nearest = nullptr;
		float Best = FLT_MAX;
		for (TActorIterator<AVEILFactionAgent> It(W); It; ++It)
		{
			AVEILFactionAgent* Agent = *It;
			if (!Agent || !Agent->IsAlive()) continue;
			const float Dist = FVector::Dist(Agent->GetActorLocation(), Runner->GetActorLocation());
			if (Dist < Best) { Best = Dist; Nearest = Agent; }
		}
		if (Nearest)
		{
			UGameplayStatics::ApplyDamage(Nearest, UPHIMito::PhiPow(30.f), Runner->GetController(), Runner,
				UDamageType::StaticClass());
			return;
		}

		// -phiwin walks the rest of the ending: artifact built, four anchors
		// aligned, the Vault answering and the win screen raised.
		if (bPhiWinProof)
		{
			for (TActorIterator<AVEILVaultAnchor> It(W); It; ++It)
			{
				It->bAligned = true;
			}
			for (TActorIterator<AVEILVault> It(W); It; ++It)
			{
				It->bArtifactBuilt = true;
				It->NotifyAnchorAligned();
				UE_LOG(LogVEIL, Log, TEXT("PHI WIN PROOF: anchors %d/4, artifact=%d, open=%d."),
					It->AlignedCount(), It->bArtifactBuilt ? 1 : 0, It->IsOpen() ? 1 : 0);
			}
		}

		UE_LOG(LogVEIL, Log, TEXT("PHI KILL PROOF: the camp is empty - nothing left to answer."));
		for (TActorIterator<AVEILVault> It(W); It; ++It)
		{
			UE_LOG(LogVEIL, Log, TEXT("PHI VAULT GATE: campCleared=%d standing=%d open=%d anchors=%d/4 artifact=%d."),
				It->bCampCleared ? 1 : 0, It->CampStanding, It->IsOpen() ? 1 : 0,
				It->AlignedCount(), It->bArtifactBuilt ? 1 : 0);
		}
	}

	// Walk him onto the drop: the pickup only counts if the runner's own body
	// touches it, so the proof uses the same overlap a player would.
	for (TActorIterator<AVEILLootPickup> It(W); It; ++It)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI KILL PROOF: %s waits at %s - walking the runner onto it."),
			*It->ItemName.ToString(), *It->GetActorLocation().ToCompactString());
		Runner->SetActorLocation(It->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		break;
	}

	UOMNIQuestSystem* Quest = W->GetSubsystem<UOMNIQuestSystem>();
	UE_LOG(LogVEIL, Log, TEXT("PHI KILL PROOF: ladder after the kill: %d XP at level %d, %d gold."),
		Quest ? Quest->XP : -1, Quest ? Quest->Level : -1, Quest ? Quest->Gold : -1);
	if (++PhiKillPhase >= 3)
	{
		W->GetTimerManager().ClearTimer(PhiKillTimer);
	}
}

void AVEILGameMode::SetPhase(EVEILGamePhase NewPhase)
{
	if (NewPhase == Phase) return;
	Phase = NewPhase;
	OnPhaseChanged.Broadcast(Phase);
	if (NewPhase == EVEILGamePhase::Resolution)
	{
		FPHIVoice::Play(GetWorld(), TEXT("V18"));
	}
}

void AVEILGameMode::HandleMysterySolved(FName MysteryId)
{
	UE_LOG(LogVEIL, Log, TEXT("Mystery solved (%s) - phase: Resolution"), *MysteryId.ToString());
	SetPhase(EVEILGamePhase::Resolution);
}

// ---------------------------------------------------------------------------
// Player Controller
// ---------------------------------------------------------------------------
AVEILPlayerController::AVEILPlayerController()
{
	// The C++ main menu is the default: a working menu with zero assets.
	MainMenuWidgetClass = UVEILMainMenu::StaticClass();
	PauseWidgetClass = UVEILPauseMenu::StaticClass();
}

static FTimerHandle GPhiTimerHandle;

void AVEILPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// Timer cross-check: a 3-second one-shot on the world's own timer
	// manager. If world time advances, PhiOnTimerCheck fires and logs
	// it; if the clock stands frozen, the TICKPROOF shows it pending.
	GetWorldTimerManager().SetTimer(GPhiTimerHandle, this, &AVEILPlayerController::PhiOnTimerCheck, 3.0f, false);

	if (HUDWidgetClass && IsLocalController())
	{
		HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget) HUDWidget->AddToViewport(0);
	}

	// THE TITLE NEVER FREEZES THE WORLD. LevelTick gates TimeSeconds, the timer
	// manager and every actor tick behind !bIsPaused, so pausing for the title
	// pinned world time at 0.00, froze every timer, and silenced the pawn and the
	// game mode alike - the whole game looked dead behind a menu that was only
	// supposed to sit on top of it. The title overlays a LIVING world (job 253).
	bool bProofRun = FParse::Param(FCommandLine::Get(), TEXT("phishow"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phikill"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phimove"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phitalk"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phimanual"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phipad"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phipadreply"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phitravel"))
		|| FParse::Param(FCommandLine::Get(), TEXT("philife"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phioptions"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phitemporal"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phistrike"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phione"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phienter"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phivoice"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phishot"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phiair"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phitour"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phiseat"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phimemory"))
		|| FParse::Param(FCommandLine::Get(), TEXT("phiquanta"));
	{
		// A VALUED SWITCH IS STILL A PROOF RUN. FParse::Param does not match
		// "-phiregion=0", so the list above missed it and the title screen took the
		// early return - which is why the runner never moved on the first two tries.
		FString ValuedProof;
		if (FParse::Value(FCommandLine::Get(), TEXT("phiregion="), ValuedProof)) { bProofRun = true; }
	}
	if (MainMenuWidgetClass && IsLocalController() && !bProofRun)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport(20);
			SetPause(false); // a living world behind the title, never a frozen one
			SetInputMode(FInputModeGameAndUI());
			bShowMouseCursor = true;
			return;
		}
	}
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	// -phiregion=<0..4>: walks the runner into a region ONE SECOND AFTER the seat
	// is placed, through the same console handler phi.goto uses - so a frame can
	// show the district the world builder actually raised, from the ground, rather
	// than the patch of open field the seat probe had to settle for.
	{
		FString RegionArg;
		// FParse::Value on "-phiregion=0": Param would not match a valued switch.
		const bool bRegionProof = FParse::Value(FCommandLine::Get(), TEXT("phiregion="), RegionArg);
		const int32 RegionIndex = bRegionProof ? FCString::Atoi(*RegionArg) : 0;
		if (bRegionProof)
		{
			FTimerHandle RegionTimer;
			GetWorldTimerManager().SetTimer(RegionTimer, FTimerDelegate::CreateWeakLambda(this, [this, RegionIndex]()
			{
				if (UWorld* W = GetWorld())
				{
					// World->Exec did NOT reach the phi.goto console command (no log, no
					// movement). Calling the same function the command calls removes the
					// indirection entirely.
					const bool bMoved = AVEILWorldBuilder::GoToRegion(W, RegionIndex);
					UE_LOG(LogVEIL, Warning, TEXT("PHI PROOF: goto region %d -> %s."),
						RegionIndex, bMoved ? TEXT("moved") : TEXT("refused"));
				}
				// 5.5 s, not 1.2: the runner is SEATED by a later timer, and the seat
				// landed on top of the goto - which is why the region readout never moved.
			}), 5.5f, false);
		}
	}

	// HEADROOM FOR THE BAKED CITY. This map builds no rooms at runtime - it is a saved world,
	// so BuildRoom's 6.4 m clear never runs here - and every shell in it was baked by an older
	// builder that laid walls at the caller's height with the roof 12 cm above. That is the low
	// ceiling the player walks into. The shells are tallened once, at boot, from their own
	// bounds: a roof near the plaza rises 260 cm, a wall grows 1.4x and rises by half of it so
	// its base stays on the ground, and the count is logged either way.
	{
		FTimerHandle HeadroomTimer;
		GetWorldTimerManager().SetTimer(HeadroomTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* W = GetWorld();
			if (!W) { return; }
			int32 Roofs = 0, Walls = 0;
			for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
			{
				AStaticMeshActor* Piece = *It;
				if (!Piece) { continue; }
				UStaticMeshComponent* Body = Piece->GetStaticMeshComponent();
				if (!Body || !Body->GetStaticMesh()) { continue; }
				const FString MeshName = Body->GetStaticMesh()->GetName();
				const FBox Box = Body->Bounds.GetBox();
				const FVector At = Box.GetCenter();
				if (FVector::Dist2D(At, FVector::ZeroVector) > 6000.f) { continue; }

				const bool bRoof = MeshName.Contains(TEXT("Roof")) || MeshName.Contains(TEXT("Ceil"));
				const bool bWall = MeshName.Contains(TEXT("Wall"));
				if (bRoof && At.Z > 150.f && At.Z < 900.f)
				{
					Piece->AddActorWorldOffset(FVector(0.f, 0.f, 260.f));
					++Roofs;
				}
				else if (bWall && At.Z < 900.f)
				{
					const FVector Old = Piece->GetActorScale3D();
					FVector Grown = Old;
					Grown.Z = Old.Z * 1.4f;
					Piece->SetActorScale3D(Grown);
					Piece->AddActorWorldOffset(FVector(0.f, 0.f, (Grown.Z - Old.Z) * 50.f));
					++Walls;
				}
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI HEADROOM: %d baked roof(s) raised 260 cm, %d wall(s) grown 1.4x - ceilings now clear a standing body."),
				Roofs, Walls);
		}), 2.5f, false);
	}

	// THE SHEET AT EYE HEIGHT. A mesh that contains the head point and is thinner than 120 cm in
	// Z is neither floor nor wall - it is a plane lying across the view, and from inside a shell
	// it reads as the ceiling resting on the player. Each one is named and hidden, then the
	// clearance over the head is traced and reported against the 180 cm body.
	{
		FTimerHandle SheetTimer;
		GetWorldTimerManager().SetTimer(SheetTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* W = GetWorld();
			APawn* Me = UGameplayStatics::GetPlayerPawn(this, 0);
			if (!W || !Me) { return; }
			const FVector Feet = Me->GetActorLocation();
			const FVector Head = Feet + FVector(0.f, 0.f, 60.f);

			int32 Sheets = 0;
			for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
			{
				AStaticMeshActor* Piece = *It;
				if (!Piece) { continue; }
				UStaticMeshComponent* Body = Piece->GetStaticMeshComponent();
				if (!Body || !Body->GetStaticMesh()) { continue; }
				const FBox Box = Body->Bounds.GetBox();
				const FVector Size = Box.GetSize();
				if (Size.Z > 120.f) { continue; }
				if (!Box.IsInsideOrOn(Head)) { continue; }
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI SHEET CULL: hid %s (%s) %.0f x %.0f x %.0f cm at z=%.0f - it lay across the eyes."),
					*Piece->GetName(), *Body->GetStaticMesh()->GetName(), Size.X, Size.Y, Size.Z, Box.GetCenter().Z);
				Piece->SetActorHiddenInGame(true);
				++Sheets;
			}

			FCollisionQueryParams Q(SCENE_QUERY_STAT(PhiCeiling), false, Me);
			float Clear = -1.f;
			const float Lifts[4] = { 0.f, 300.f, 900.f, 1600.f };
			for (int32 i = 0; i < 4; ++i)
			{
				FHitResult Hit;
				const FVector Start = Feet + FVector(0.f, 0.f, Lifts[i]);
				if (W->LineTraceSingleByChannel(Hit, Start, Start + FVector(0.f, 0.f, 4000.f), ECC_Visibility, Q))
				{
					Clear = Hit.ImpactPoint.Z - Feet.Z;
					break;
				}
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI CEILING: %d sheet(s) culled, roof now %s over the runner at (%.0f, %.0f) - standing body 180 cm."),
				Sheets,
				Clear < 0.f ? TEXT("OPEN SKY") : *FString::Printf(TEXT("%.0f cm"), Clear),
				Feet.X, Feet.Y);

			// Every roof in the plaza is probed from under its own bottom edge down to the floor
			// beneath it: that gap IS the interior's clear height, measured rather than assumed.
			int32 Probed = 0, Low = 0;
			float MinClear = 1.0e9f;
			for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
			{
				AStaticMeshActor* Piece = *It;
				if (!Piece) { continue; }
				UStaticMeshComponent* Body = Piece->GetStaticMeshComponent();
				if (!Body || !Body->GetStaticMesh()) { continue; }
				const FString MeshName = Body->GetStaticMesh()->GetName();
				if (!MeshName.Contains(TEXT("Roof")) && !MeshName.Contains(TEXT("Ceil"))) { continue; }
				const FBox Box = Body->Bounds.GetBox();
				const FVector At = Box.GetCenter();
				if (FVector::Dist2D(At, FVector::ZeroVector) > 6000.f) { continue; }
				FHitResult FloorHit;
				const float Down = Box.Min.Z - 5.f;
				if (!W->LineTraceSingleByChannel(FloorHit, FVector(At.X, At.Y, Down),
					FVector(At.X, At.Y, Down - 3000.f), ECC_Visibility, Q)) { continue; }
				const float Gap = Down - FloorHit.ImpactPoint.Z;
				if (Gap <= 0.f || Gap > 2000.f) { continue; }
				MinClear = FMath::Min(MinClear, Gap);
				++Probed;
				if (Gap < 260.f) { ++Low; }
			}
			if (Probed > 0)
			{
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI INTERIOR: %d roof(s) probed, min clear %.0f cm, %d under 260 cm - verdict %s."),
					Probed, MinClear, Low, Low == 0 ? TEXT("EVERY INTERIOR CLEARS A BODY") : TEXT("STILL CRAMPED"));
			}
			else
			{
				UE_LOG(LogVEIL, Warning, TEXT("PHI INTERIOR: no roof above a floor was found in the plaza - nothing to probe."));
			}
		}), 3.5f, false);
	}

	// -phitype: THE CLICK, WITHOUT A MOUSE. The proof run opens the field through
	// OpenTypeToONE - the click handler's own door - and sends through SubmitTypedLine, the
	// door Enter uses. Nothing here bypasses the field; it exercises it.
	if (FParse::Param(FCommandLine::Get(), TEXT("phitype")))
	{
		FTimerHandle TypeOpen;
		GetWorldTimerManager().SetTimer(TypeOpen, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (UWorld* W = GetWorld())
			{
				if (AVEILPlayerController* TypePC = Cast<AVEILPlayerController>(W->GetFirstPlayerController()))
				{
					TypePC->OpenTypeToONE();
					UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE PROOF: field opened through OpenTypeToONE."));
				}
			}
		}), 4.f, false);

		FTimerHandle TypeSend;
		GetWorldTimerManager().SetTimer(TypeSend, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (UWorld* W = GetWorld())
			{
				if (AVEILPlayerController* TypePC = Cast<AVEILPlayerController>(W->GetFirstPlayerController()))
				{
					TypePC->SubmitTypedLine(TEXT("fly me to the keeper"));
					TypePC->CloseTypeToONE();
					UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE PROOF: line submitted and field closed."));
				}
			}
		}), 7.f, false);
	}

	// -phione: speaks to ONE through the same Say() a spoken sentence reaches,
	// so the proof run proves the skill, the hand and the ledger - not a shortcut.
	if (FParse::Param(FCommandLine::Get(), TEXT("phione")))
	{
		FTimerHandle OneTimer;
		GetWorldTimerManager().SetTimer(OneTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (UWorld* W = GetWorld())
			{
				if (UPHIONE* One = W->GetSubsystem<UPHIONE>())
				{
					One->Say(TEXT("ONE, fly me to the keeper"));
				}
			}
		}), 3.0f, false);
	}

	// -phienter: stands the runner INSIDE a building - through the door the builder
	// cut, not on top of the roof - so "enterable" is a frame, not a claim.
	if (FParse::Param(FCommandLine::Get(), TEXT("phienter")))
	{
		FTimerHandle EnterTimer;
		GetWorldTimerManager().SetTimer(EnterTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* W = GetWorld();
			APawn* P = UGameplayStatics::GetPlayerPawn(W, 0);
			if (!W || !P) { return; }
			AVEILWorldBuilder* Builder = nullptr;
			for (TActorIterator<AVEILWorldBuilder> It(W); It; ++It) { Builder = *It; break; }
			if (!Builder || Builder->RoomsBuilt == 0)
			{
				UE_LOG(LogVEIL, Warning, TEXT("PHI ENTER PROOF: no rooms were built - nothing to enter."));
				return;
			}
			P->SetActorLocation(Builder->FirstRoomInterior, false, nullptr, ETeleportType::TeleportPhysics);
			if (AController* C = P->GetController())
			{
				// Face the HEARTH, not the door. Standing at the threshold looking out
				// photographs the street and proves nothing about the room.
				const FVector ToDoor = (Builder->FirstRoomDoor - Builder->FirstRoomInterior).GetSafeNormal2D();
				C->SetControlRotation(FRotator(-4.f, (-ToDoor).Rotation().Yaw, 0.f));
			}
			if (AVEILCharacter* Runner = Cast<AVEILCharacter>(P)) { Runner->SetClimbAxis(0.f); }
			UE_LOG(LogVEIL, Warning, TEXT("PHI ENTER PROOF: runner standing inside a building at %s (rooms=%d)."),
				*P->GetActorLocation().ToCompactString(), Builder->RoomsBuilt);

			// Is the room actually enclosed? Four horizontal traces from the runner's
			// chest answer with actor and mesh names instead of an opinion.
			const FVector Eye = P->GetActorLocation() + FVector(0.f, 0.f, 60.f);
			const FVector Dirs[4] = { FVector(1,0,0), FVector(-1,0,0), FVector(0,1,0), FVector(0,-1,0) };
			const TCHAR* Names[4] = { TEXT("+X"), TEXT("-X"), TEXT("+Y"), TEXT("-Y") };
			for (int32 i = 0; i < 4; ++i)
			{
				FHitResult Hit;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(PHIEnterWall), false, P);
				if (W->LineTraceSingleByChannel(Hit, Eye, Eye + Dirs[i] * 900.f, ECC_Visibility, Params))
				{
					const AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Hit.GetActor());
					UE_LOG(LogVEIL, Warning, TEXT("PHI ENTER WALL %s: %s mesh=%s at %.0f cm"),
						Names[i], Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("none"),
						SMA && SMA->GetStaticMeshComponent() && SMA->GetStaticMeshComponent()->GetStaticMesh()
							? *SMA->GetStaticMeshComponent()->GetStaticMesh()->GetName() : TEXT("none"),
						Hit.Distance);
				}
				else
				{
					UE_LOG(LogVEIL, Warning, TEXT("PHI ENTER WALL %s: NOTHING within 900 cm"), Names[i]);
				}
			}
		}), 4.2f, false);
	}

	// -phimanual: opens the controls page for a still frame, so the same page a
	// pad player reaches with D-PAD LEFT is provable without pressing anything.
	if (FParse::Param(FCommandLine::Get(), TEXT("phimanual")))
	{
		bShowFieldManual = true;
	}

	// -phipad: pushes a real gamepad key event through the controller's own
	// InputKey, so the pad binding and the device switch are proven by the frame
	// they produce - not merely by the mapping existing in the map.
	if (FParse::Param(FCommandLine::Get(), TEXT("phipad")))
	{
		// Three real presses, in order: pad opens the page, keyboard closes it,
		// pad opens it again. Each line reports what the game did, so both the
		// pad routing and the device swap are proven, not assumed.
		auto Press = [this](const FKey& K, const TCHAR* Label)
		{
			InputKey(FInputKeyEventArgs::CreateSimulated(K, IE_Pressed, 1.f));
			UE_LOG(LogVEIL, Log, TEXT("PHI PAD PROOF %s: manual=%d device=%s"),
				Label, bShowFieldManual ? 1 : 0,
				VEILInput::IsGamepad() ? TEXT("gamepad") : TEXT("keyboard"));
		};
		static FTimerHandle GPad1, GPad2, GPad3;
		GetWorldTimerManager().SetTimer(GPad1, FTimerDelegate::CreateWeakLambda(this,
			[Press]() { Press(EKeys::Gamepad_DPad_Left, TEXT("pad d-pad left")); }), 6.f, false);
		GetWorldTimerManager().SetTimer(GPad2, FTimerDelegate::CreateWeakLambda(this,
			[Press]() { Press(EKeys::F1, TEXT("keyboard F1")); }), 9.f, false);
		GetWorldTimerManager().SetTimer(GPad3, FTimerDelegate::CreateWeakLambda(this,
			[Press]() { Press(EKeys::Gamepad_DPad_Left, TEXT("pad d-pad left again")); }), 12.f, false);
		// Then close it again, so the frame after this proves the STEP CARD in pad
		// mode: same gesture, glyph swapped to the controller.
		static FTimerHandle GPad4;
		GetWorldTimerManager().SetTimer(GPad4, FTimerDelegate::CreateWeakLambda(this,
			[Press]() { Press(EKeys::Gamepad_DPad_Left, TEXT("pad d-pad left closes")); }), 15.f, false);
	}

	// -phipadreply: END TO END ON THE PAD. Speaks to the nearest statue, walks the
	// lines until the replies are on the table, then presses A through the real
	// InputKey route - the same call a controller makes - and logs the answer the
	// world actually applied. A reply is the one thing a pad could never do before.
	if (FParse::Param(FCommandLine::Get(), TEXT("phipadreply")))
	{
		static FTimerHandle GPadReply;
		GetWorldTimerManager().SetTimer(GPadReply, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UVEILDialogue* Dlg = GetWorld() ? GetWorld()->GetSubsystem<UVEILDialogue>() : nullptr;
			if (!Dlg) { return; }

			TalkToNearest();
			for (int32 Step = 0; Step < 12 && Dlg->IsActive() && !Dlg->IsChoosing(); ++Step)
			{
				Dlg->Advance();
			}
			UE_LOG(LogVEIL, Log, TEXT("PHI PAD REPLY: speaker=%s choices=%d open=%d"),
				*Dlg->GetSpeakerName(), Dlg->GetChoices().Num(), Dlg->IsChoosing() ? 1 : 0);

			if (!Dlg->IsChoosing()) { return; }

			// A = reply 1, through the controller's own key route.
			InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::Gamepad_FaceButton_Bottom, IE_Pressed, 1.f));

			// The press is dispatched on the input tick, not on the call itself, so
			// the result is read one frame later - reading it here reported 0 while
			// the world had already answered, which is a lie about a working key.
			static FTimerHandle GPadReplyAfter;
			GetWorldTimerManager().SetTimer(GPadReplyAfter, FTimerDelegate::CreateWeakLambda(this, [this, Dlg]()
			{
				UE_LOG(LogVEIL, Log, TEXT("PHI PAD REPLY: after pad A - answered=%d consequence=%s"),
					Dlg->HasAnswered() ? 1 : 0, *Dlg->GetConsequence());
			}), 0.4f, false);
		}), 8.f, false);
	}

	// -phiquanta: holds the temporal field open so the superposition itself can
	// be seen in a still frame instead of resolving between screenshots.
	if (FParse::Param(FCommandLine::Get(), TEXT("phiquanta")))
	{
		if (UVEILTime* Clock = GetWorld()->GetSubsystem<UVEILTime>())
		{
			Clock->SetAwaitingObserver(true);
			Clock->ForceField();
			UE_LOG(LogVEIL, Log, TEXT("PHI TEMPORAL: proof hold - the field will wait for an observer."));
		}
	}

	// -phitemporal: opens the timeless window and holds it, so one still frame can
	// show the runner standing a beat ahead of the world, with the world slowed to
	// 1/phi^drift beneath him.
	if (FParse::Param(FCommandLine::Get(), TEXT("phitemporal")))
	{
		if (UVEILTime* Clock = GetWorld()->GetSubsystem<UVEILTime>())
		{
			Clock->ForcePhase(2);
			Clock->SetHoldPhase(true);
		}
	}

	// -phistrike: presses the strike key the moment the temporal field is open, so
	// the live path - key, swing, observation, collapse onto a future beat, world
	// catching up - is proven in a run with no hand on the keyboard. The press is
	// a simulated key event through the controller, exactly like the pad proof.
	if (FParse::Param(FCommandLine::Get(), TEXT("phistrike")))
	{
		static int32 ProofStrikes = 0;
		static FTimerHandle GStrikeTimer;
		GetWorldTimerManager().SetTimer(GStrikeTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			const UVEILTime* Clock = GetWorld() ? GetWorld()->GetSubsystem<UVEILTime>() : nullptr;
			AVEILCharacter* Runner = Cast<AVEILCharacter>(GetPawn());
			if (!Clock || !Runner || !Clock->IsSuperposed() || ProofStrikes >= 3) { return; }
			// Read the field before the gesture resolves it, then run the same
			// gesture the F key runs - not a shortcut past it.
			++ProofStrikes;
			UE_LOG(LogVEIL, Log, TEXT("PHI STRIKE PROOF %d: field %d futures wide, coherence %.0f%% - pressing F."),
				ProofStrikes, Clock->Candidates().Num(), Clock->GetCoherence() * 100.f);
			Runner->Strike();
		}), 0.5f, true);
	}

	// -phioptions: opens the options page for a still frame.
	if (FParse::Param(FCommandLine::Get(), TEXT("phioptions")))
	{
		OpenOptions();
	}

	// -philife: stands the runner in a mixed archetype and opens the living-world
	// page, so one frame shows the clock, the lives and the no-lock rule.
	if (FParse::Param(FCommandLine::Get(), TEXT("philife")))
	{
		SelectArchetype(1); // STORMCALL on a runner who already holds every other
		bShowChart = true;
		ChartPage = 1;
		UE_LOG(LogVEIL, Log, TEXT("PHI SOCIETY: %d archetypes open to the runner, none locked."),
			VEILSociety::Archetypes().Num());
		UE_LOG(LogVEIL, Log, TEXT("PHI SOCIETY: %d people carry a life, freely mixed."),
			VEILSociety::Lives().Num());
	}

	// -phitravel: opens the chart, travels to the highlighted beacon, then reopens
	// the chart on the arrival point. The log carries the before/after positions.
	if (FParse::Param(FCommandLine::Get(), TEXT("phitravel")))
	{
		static FTimerHandle GT1, GT2, GT3;
		GetWorldTimerManager().SetTimer(GT1, FTimerDelegate::CreateWeakLambda(this,
			[this]() { ToggleChart(); }), 6.f, false);
		GetWorldTimerManager().SetTimer(GT2, FTimerDelegate::CreateWeakLambda(this,
			[this]()
			{
				const FVector Before = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
				// Aim at the First Beacon by name, so the proof is a journey home and
				// not a hop to the lamp he happens to be standing beside.
				if (UVEILFastTravel* FT = GetWorld() ? GetWorld()->GetSubsystem<UVEILFastTravel>() : nullptr)
				{
					for (int32 i = 0; i < FT->Waypoints.Num(); ++i)
					{
						if (FT->Waypoints[i].bLit && i != FT->StandingAtIndex(Before)) { ChartSel = i; break; }
					}
				}
				ChartTravel();
				const FVector After = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
				UE_LOG(LogVEIL, Log, TEXT("PHI TRAVEL PROOF: runner moved %.0f cm (%s -> %s)."),
					FVector::Dist(Before, After), *Before.ToCompactString(), *After.ToCompactString());
			}), 9.f, false);
		GetWorldTimerManager().SetTimer(GT3, FTimerDelegate::CreateWeakLambda(this,
			[this]() { if (!bShowChart) { ToggleChart(); } }), 12.f, false);
	}

	// -phimemory: files a deed, then opens the same conversation - the exact two
	// calls a player's reply makes (NoteDeed, then Begin). The opening line in the
	// frame is therefore the one that remembers, not the authored greeting.
	if (FParse::Param(FCommandLine::Get(), TEXT("phimemory")))
	{
		static FTimerHandle GPhiMemoryTimer;
		GetWorldTimerManager().SetTimer(GPhiMemoryTimer,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (UPHILivingWorld* Living = GetWorld() ? GetWorld()->GetSubsystem<UPHILivingWorld>() : nullptr)
				{
					Living->NoteDeed(FName(TEXT("sera_vahl")), true, TEXT("Then I will be on time."));
				}
				if (UVEILDialogue* Dlg = GetWorld() ? GetWorld()->GetSubsystem<UVEILDialogue>() : nullptr)
				{
					if (Dlg->Begin(FName(TEXT("sera_vahl"))))
					{
						UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: memory opening - \"%s\""), *Dlg->GetFullText());
					}
				}
			}), 3.f, false);
	}

	// -phitalk: opens a real conversation through the same path [E] uses, so a
	// live frame proves the cast, the box and the typewriter without a keyboard.
	if (FParse::Param(FCommandLine::Get(), TEXT("phitalk")))
	{
		static FTimerHandle GPhiTalkTimer;
		GetWorldTimerManager().SetTimer(GPhiTalkTimer,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (UVEILDialogue* Dlg = GetWorld() ? GetWorld()->GetSubsystem<UVEILDialogue>() : nullptr)
				{
					if (Dlg->Begin(FName(TEXT("sera_vahl"))))
					{
						UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: %s opened in a live frame."),
							*Dlg->GetSpeakerName());
					}
				}
			}), 2.5f, false);
	}

	// -phibeacons: forge all three beacons in-game, before the photo, so the proof
	// frame is taken BY THE GAME at a settled exposure instead of through the
	// console. Typing the claim into the console opened a bright panel, the eye
	// adaptation pulled the whole frame down to match it, and the capture came back
	// as a near-black photo of a perfectly lit world - the "it keeps going black"
	// frames were that, not the scene. Same function a player's interact key calls.
	if (FParse::Param(FCommandLine::Get(), TEXT("phibeacons")))
	{
		FTimerHandle GB1;
		GetWorldTimerManager().SetTimer(GB1, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (UOMNIQuestSystem* Quest = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
			{
				for (int32 i = 0; i < 3; ++i) { Quest->ClaimBeacon(i); }
				UE_LOG(LogVEIL, Warning, TEXT("PHI BEACON PROOF: %d of 3 forged in a live frame (no console)."),
					Quest->ClaimedBeacons.Num());
			}
		}), 5.f, false);
	}

	// -phishot: frames written from inside the game at a few beats. Used to look
	// at the interface itself (a screen grab of a flip-model swapchain is black).
	if (FParse::Param(FCommandLine::Get(), TEXT("phishot")))
	{
		static FTimerHandle GS1, GS3;
		auto Shoot = [this]()
		{
			if (APlayerController* ShotPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
			{
				// FACE THE CITY BEFORE THE SHUTTER. The runner's seat looks out across the
				// open plain, so the player's own camera was aimed at three kilometres of
				// fog and the district sat behind his shoulder - the frame proved an empty
				// horizon, not a city. This is not a different camera: it is the same eyes,
				// at the same spawn, turned toward the Spire the frame exists to show.
				if (const APawn* Body = ShotPC->GetPawn())
				{
					const FVector Eye = Body->GetActorLocation();
					const FRotator Toward = (FVector(0.f, 0.f, 900.f) - Eye).Rotation();
					ShotPC->SetControlRotation(FRotator(-6.f, Toward.Yaw, 0.f));

					// SAY WHERE THE EYES ARE AND WHAT IS IN FRONT OF THEM. A frame of empty
					// horizon has three possible causes - a camera facing the wrong way, a
					// camera inside a wall, or no world at all - and they need three different
					// fixes. The picture cannot tell them apart; these two traces can.
					if (UWorld* ShotWorld = GetWorld())
					{
						const FVector Fwd = ShotPC->GetControlRotation().Vector();
						FHitResult Down, Ahead;
						// NOT THE RUNNER'S OWN CAPSULE. The eye sits at the body's origin, so an
						// unignored trace answered "floor Actor_0 at 0 cm; ahead Actor_0 at 0 cm"
						// every single frame - a probe reporting itself instead of the room.
						FCollisionQueryParams EyeParams(SCENE_QUERY_STAT(VeilShotEyes), false);
						if (Body) { EyeParams.AddIgnoredActor(Body); }
						// A MESHLESS PROXY IS NOT WHAT THE CAMERA SEES.
						//
						// The hub's landmarks are collision boxes carrying no geometry, and the first
						// trace out of the runner's eyes stopped on one every frame: "floor Actor_0 at
						// 0 cm; ahead Actor_0 at 0 cm" was the probe reporting a box the player cannot
						// see, while the wall in front of the lens went unnamed. Each hit is checked for
						// a real mesh; a proxy is ignored and the trace is asked again.
						auto TraceForMesh = [&ShotWorld, &Eye, &EyeParams](FHitResult& Out, const FVector& To)
						{
							for (int32 Try = 0; Try < 6; ++Try)
							{
								if (!ShotWorld->LineTraceSingleByChannel(Out, Eye, To, ECC_Visibility, EyeParams))
								{
									return;
								}
								AActor* HitActor = Out.GetActor();
								if (!HitActor) { return; }
								const UStaticMeshComponent* HitMesh =
									HitActor->FindComponentByClass<UStaticMeshComponent>();
								if (HitMesh && HitMesh->GetStaticMesh()) { return; }
								EyeParams.AddIgnoredActor(HitActor);
							}
						};
						TraceForMesh(Down, Eye - FVector(0.f, 0.f, 20000.f));
						TraceForMesh(Ahead, Eye + Fwd * 3000.f);
						UE_LOG(LogVEIL, Warning,
							TEXT("VEIL SHOT EYES: body V(X=%.0f, Y=%.0f, Z=%.0f) fwd V(X=%.2f, Y=%.2f, Z=%.2f); floor %s at %.0f cm; ahead %s at %.0f cm."),
							Eye.X, Eye.Y, Eye.Z, Fwd.X, Fwd.Y, Fwd.Z,
							Down.bBlockingHit ? *Down.GetActor()->GetName() : TEXT("NONE"), Down.Distance,
							Ahead.bBlockingHit ? *Ahead.GetActor()->GetName() : TEXT("NONE"), Ahead.Distance);
					}
				}
				ShotPC->ConsoleCommand(TEXT("HighResShot 1920x1080"), true);
			}
		};
		GetWorldTimerManager().SetTimer(GS1, FTimerDelegate::CreateWeakLambda(this, [Shoot]() { Shoot(); }), 7.f, false);
		// THE SECOND FRAME COMES FROM THE SAME EYES, NOT FROM THE SKY. The aerial beat
		// that used to run here pointed the camera actor at 90 m up, and from there the
		// district's upright buildings read as flat slabs lying on a plane - a camera
		// fault that looked exactly like a broken world, in the photo the player is
		// asked to judge. The proof is the frame the player actually sees, so both
		// beats stand on the runner's own camera and the same spawn.
		GetWorldTimerManager().SetTimer(GS3, FTimerDelegate::CreateWeakLambda(this, [Shoot]() { Shoot(); }), 15.f, false);
	}

	// -phiair: THE GROUND, PHOTOGRAPHED FROM ABOVE.
	//
	// -phishot deliberately stands on the runner's own eyes, because that is the frame a
	// player judges. It cannot answer the other question - whether the HUB has courses
	// cut into the hill, whether the west cliff is falling, whether the Spire stands on
	// the skyline - because from a market every one of those is behind a stall. So this
	// beat lifts the same pawn over the town, twice, and says where the eyes are in the
	// log beside the frame.
	if (FParse::Param(FCommandLine::Get(), TEXT("phiair")))
	{
		// Say it out loud at the moment the flag is read: a silent block and a block
		// that never ran look identical in a log, and that ambiguity has cost this
		// project whole afternoons before.
		UE_LOG(LogVEIL, Warning, TEXT("VEIL AIR: armed - two frames over the hub, 7s and 16s after the world."));
		static FTimerHandle GA1, GA2, GA3, GA4;
		auto Air = [this](const FVector& At, const FVector& LookAt, const TCHAR* Label)
		{
			APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
			APawn* Body = PC ? PC->GetPawn() : nullptr;
			if (!PC || !Body) { return; }
			Body->SetActorLocation(At, false, nullptr, ETeleportType::TeleportPhysics);
			const FRotator Toward = (LookAt - At).Rotation();
			PC->SetControlRotation(FRotator(-14.f, Toward.Yaw, 0.f));
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL AIR: %s - eyes at V(X=%.0f, Y=%.0f, Z=%.0f) looking at V(X=%.0f, Y=%.0f, Z=%.0f)."),
				Label, At.X, At.Y, At.Z, LookAt.X, LookAt.Y, LookAt.Z);
		};
		auto ShootAir = [this]()
		{
			if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
			{
				PC->ConsoleCommand(TEXT("HighResShot 1920x1080"), true);
				UE_LOG(LogVEIL, Warning, TEXT("VEIL AIR: frame written from the air."));
			}
		};

		// North of the plaza, high enough to clear the roofs: the terraced courses and
		// the market they surround.
		GetWorldTimerManager().SetTimer(GA1, FTimerDelegate::CreateWeakLambda(this,
			[Air]() { Air(FVector(0.f, -7600.f, 3400.f), FVector(0.f, 0.f, 300.f), TEXT("the hub from the north")); }),
			7.f, false);
		GetWorldTimerManager().SetTimer(GA2, FTimerDelegate::CreateWeakLambda(this,
			[ShootAir]() { ShootAir(); }), 9.f, false);

		// West of the cliff, looking east across the falls to the plaza and the Spire.
		GetWorldTimerManager().SetTimer(GA3, FTimerDelegate::CreateWeakLambda(this,
			[Air]() { Air(FVector(-8200.f, -3600.f, 3000.f), FVector(600.f, 0.f, 700.f), TEXT("the west cliff and the Spire")); }),
			16.f, false);
		GetWorldTimerManager().SetTimer(GA4, FTimerDelegate::CreateWeakLambda(this,
			[ShootAir]() { ShootAir(); }), 18.f, false);
	}

	// -phiseat: WHAT IS THE RUNNER STANDING IN?
	//
	// The Play frame at the seat showed the runner pressed into a gold mass with the
	// camera inside it. An editor-side probe found NOTHING within six metres, because
	// the offender is spawned at Play, not saved in the level - so the question has to
	// be asked of the running world, and answered with distances rather than a guess.
	if (FParse::Param(FCommandLine::Get(), TEXT("phiseat")))
	{
		FTimerHandle SeatTimer;
		GetWorldTimerManager().SetTimer(SeatTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* W = GetWorld();
			const APawn* Body = GetPawn();
			if (!W || !Body) { return; }
			const FVector At = Body->GetActorLocation();
			int32 Found = 0;
			float Nearest = 1.0e9f;
			FString NearestName;
			for (TActorIterator<AActor> It(W); It; ++It)
			{
				AActor* A = *It;
				if (!A || A == Body) { continue; }
				const float D = (float)FVector::Dist(A->GetActorLocation(), At);
				if (D > 900.f) { continue; }
				++Found;
				FString MeshName = TEXT("-");
				if (const AStaticMeshActor* SM = Cast<AStaticMeshActor>(A))
				{
					if (const UStaticMeshComponent* C = SM->GetStaticMeshComponent())
					{
						if (const UStaticMesh* M = C->GetStaticMesh()) { MeshName = M->GetName(); }
					}
				}
				UE_LOG(LogVEIL, Warning,
					TEXT("VEIL SEAT: %.0f cm %s (%s) at V(%.0f,%.0f,%.0f) mesh=%s"),
					D, *A->GetName(), *A->GetClass()->GetName(), A->GetActorLocation().X,
					A->GetActorLocation().Y, A->GetActorLocation().Z, *MeshName);
				if (D < Nearest) { Nearest = D; NearestName = A->GetName(); }
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL SEAT: %d actor(s) within 900 cm of V(%.0f,%.0f,%.0f); nearest %s at %.0f cm."),
				Found, At.X, At.Y, At.Z, *NearestName, Nearest);
		}), 5.0f, false);
	}

	// -phitour: FIVE REGIONS, ONE SESSION.
	//
	// -phiregion proves one arrival. A world is playable when ALL of it can be reached
	// and stood on, so this beat walks the five region pads three seconds apart through
	// the same GoToRegion the phi.goto command calls, and under each arrival it traces
	// DOWN for a floor. A region whose pad has no floor is the failure that dropped the
	// runner to Z = -5000 once already, and a skyline photograph cannot show it.
	if (FParse::Param(FCommandLine::Get(), TEXT("phitour")))
	{
		UE_LOG(LogVEIL, Warning, TEXT("VEIL TOUR: armed - five regions, three seconds apart, floor traced at each."));
		static FTimerHandle GTour[5];
		static const TCHAR* TourName[5] =
		{
			TEXT("Hub"), TEXT("Nest"), TEXT("Spire"), TEXT("Omni District"), TEXT("Deep")
		};
		for (int32 R = 0; R < 5; ++R)
		{
			GetWorldTimerManager().SetTimer(GTour[R], FTimerDelegate::CreateWeakLambda(this, [this, R]()
			{
				UWorld* W = GetWorld();
				if (!W) { return; }
				const bool bMoved = AVEILWorldBuilder::GoToRegion(W, R);
				const APawn* Body = GetPawn();
				const FVector At = Body ? Body->GetActorLocation() : FVector::ZeroVector;
				FHitResult Down;
				W->LineTraceSingleByChannel(Down, At + FVector(0.f, 0.f, 300.f),
					At - FVector(0.f, 0.f, 20000.f), ECC_Visibility);
				UE_LOG(LogVEIL, Warning,
					TEXT("VEIL TOUR %d/5 %s: %s at V(X=%.0f, Y=%.0f, Z=%.0f); floor %s at %.0f cm."),
					R + 1, TourName[R], bMoved ? TEXT("moved") : TEXT("refused"), At.X, At.Y, At.Z,
					Down.bBlockingHit ? *Down.GetActor()->GetName() : TEXT("NONE"),
					Down.bBlockingHit ? Down.Distance : -1.f);
			}), 5.5f + 3.f * (float)R, false);
		}
	}

	// -phivoice: the whole natural-talk contract, in one live frame, through the
	// same calls a player's hands make. Same person twice (second opening must
	// differ and carry the deed), then a construct (must be cool, not monotone),
	// then the district talking to itself with nobody asking it to.
	if (FParse::Param(FCommandLine::Get(), TEXT("phivoice")))
	{
		static FTimerHandle GV1, GV2, GV3, GV4, GV5, GV6, GV7, GV8;
		auto Dlg = [this]() -> UVEILDialogue*
		{
			return GetWorld() ? GetWorld()->GetSubsystem<UVEILDialogue>() : nullptr;
		};
		// The photo is taken BY THE GAME, not by a screen grab: a flip-model
		// swapchain under 300% desktop scaling came back black from PrintWindow and
		// CopyFromScreen, so the frame is rendered and written from inside.
		auto Shot = [this]()
		{
			if (APlayerController* ShotPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
			{
				ShotPC->ConsoleCommand(TEXT("HighResShot 1920x1080"), true);
				UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: frame written to Saved/Screenshots."));
			}
		};
		auto Speak = [this, Dlg](FName Who)
		{
			UVEILDialogue* D = Dlg();
			if (!D || !D->Begin(Who)) { return; }
			const FVEILVoiceTake& V = D->GetVoice();
			UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: visit %d of %s - \"%s\""),
				D->TalkCount(Who), *D->GetSpeakerName(), *D->GetFullText());
			UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: take %s%s"), *V.Mood,
				V.bMachine ? TEXT(" (machine)") : TEXT(""));
		};

		GetWorldTimerManager().SetTimer(GV1, FTimerDelegate::CreateWeakLambda(this,
			[Speak]() { Speak(FName(TEXT("sera_vahl"))); }), 3.f, false);

		// Walk the greeting to the choice, the way [SPACE] does.
		GetWorldTimerManager().SetTimer(GV2, FTimerDelegate::CreateWeakLambda(this,
			[Dlg]()
			{
				UVEILDialogue* D = Dlg();
				if (!D) { return; }
				for (int32 i = 0; i < 40 && !D->IsChoosing() && D->IsActive(); ++i) { D->Advance(); }
				UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: %d repl%s on the table, choosing=%d."),
					D->GetChoices().Num(), D->GetChoices().Num() == 1 ? TEXT("y") : TEXT("ies"),
					D->IsChoosing() ? 1 : 0);
				if (D->IsChoosing())
				{
					const bool bTook = D->Choose(0);
					UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: reply taken=%d  %s"),
						bTook ? 1 : 0, *D->GetConsequence());
				}
			}), 4.f, false);

		// Second conversation with the SAME person: visit 2, memory first.
		GetWorldTimerManager().SetTimer(GV3, FTimerDelegate::CreateWeakLambda(this,
			[Speak, Dlg, Shot]()
			{
				if (UVEILDialogue* D = Dlg()) { D->End(); }
				Speak(FName(TEXT("sera_vahl")));
				Shot();
			}), 6.4f, false);

		// Then a construct: the machine take has to exist and be narrow.
		GetWorldTimerManager().SetTimer(GV4, FTimerDelegate::CreateWeakLambda(this,
			[Speak, Dlg, Shot]()
			{
				if (UVEILDialogue* D = Dlg()) { D->End(); }
				Speak(FName(TEXT("unit_ithaca")));
				Shot();
			}), 9.f, false);

		// Hold the frame so the district gets its own exchange without being asked.
		GetWorldTimerManager().SetTimer(GV5, FTimerDelegate::CreateWeakLambda(this,
			[this, Shot]()
			{
				if (const UPHILivingWorld* Living = GetWorld() ? GetWorld()->GetSubsystem<UPHILivingWorld>() : nullptr)
				{
					UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: %d exchange(s) overheard - \"%s\""),
						Living->ExchangeCount(), *Living->OverheardLine());
				}
				Shot();
			}), 26.f, false);

		// THE OTHER HALF OF MEMORY: an ANGRY answer. Refusing somebody is a deed
		// too, and their next opening is not the warm one.
		GetWorldTimerManager().SetTimer(GV6, FTimerDelegate::CreateWeakLambda(this,
			[Dlg]()
			{
				UVEILDialogue* D = Dlg();
				if (!D) { return; }
				D->End();
				if (!D->Begin(FName(TEXT("ilva_marrow")))) { return; }
				for (int32 i = 0; i < 40 && !D->IsChoosing() && D->IsActive(); ++i) { D->Advance(); }
				if (D->IsChoosing())
				{
					const bool bTook = D->Choose(1); // the refusal, not the purchase
					UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: defied ilva_marrow, taken=%d  %s"),
						bTook ? 1 : 0, *D->GetConsequence());
				}
			}), 12.f, false);

		GetWorldTimerManager().SetTimer(GV7, FTimerDelegate::CreateWeakLambda(this,
			[this, Dlg]()
			{
				UVEILDialogue* D = Dlg();
				if (!D) { return; }
				D->End();
				if (!D->Begin(FName(TEXT("ilva_marrow")))) { return; }
				UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: after defying her, ILVA MARROW opens - \"%s\""),
					*D->GetFullText());
				UE_LOG(LogVEIL, Log, TEXT("PHI TALK PROOF: her take is %s"), *D->GetVoice().Mood);
			}), 15.f, false);

		GetWorldTimerManager().SetTimer(GV8, FTimerDelegate::CreateWeakLambda(this,
			[Dlg]() { if (UVEILDialogue* D = Dlg()) { D->End(); } }), 27.f, false);
	}
}

void AVEILPlayerController::TalkToNearest()
{
	UWorld* W = GetWorld();
	if (!W) { return; }
	UVEILDialogue* Dlg = W->GetSubsystem<UVEILDialogue>();
	if (!Dlg) { return; }

	// A waiting choice owns the button: while it is open, X is reply 2 and E
	// does nothing, so the answer cannot be skipped by the same press.
	if (Dlg->IsChoosing()) { return; }

	// One key, two jobs: if someone is already talking, this is "next line".
	if (Dlg->IsActive()) { Dlg->Advance(); return; }

	const APawn* P = GetPawn();
	const FVector At = P ? P->GetActorLocation() : FVector::ZeroVector;
	FName Best = NAME_None;
	float BestDist = 1600.f;

	// The three talking statues are three specific people.
	for (TActorIterator<AOMNIQuestGiver> It(W); It; ++It)
	{
		const float D = (float)FVector::Dist(At, It->GetActorLocation());
		if (D < BestDist) { BestDist = D; Best = VEILCast::CharacterForGiver(It->GiverName); }
	}
	// A machine speaks for its own faction, deterministically per machine.
	for (TActorIterator<AVEILFactionAgent> It(W); It; ++It)
	{
		const float D = (float)FVector::Dist(At, It->GetActorLocation());
		if (D < BestDist) { BestDist = D; Best = VEILCast::CharacterForFaction(It->Faction, It->GetUniqueID()); }
	}

	if (Best != NAME_None)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI DIALOGUE: speaking to %s at %.0f cm."), *Best.ToString(), BestDist);
		Dlg->Begin(Best);
		// Speaking is an observation of the temporal field.
		if (UVEILTime* Clock = W->GetSubsystem<UVEILTime>()) { Clock->Observe(TEXT("speak")); }
	}
	else
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI DIALOGUE: nobody within earshot."));
	}
}

void AVEILPlayerController::AdvanceDialogue()
{
	if (UVEILDialogue* Dlg = GetWorld() ? GetWorld()->GetSubsystem<UVEILDialogue>() : nullptr)
	{
		if (Dlg->IsActive()) { Dlg->Advance(); }
	}
}

void AVEILPlayerController::ToggleOS()
{
	if (UVEILOS* OS = GetWorld() ? GetWorld()->GetSubsystem<UVEILOS>() : nullptr)
	{
		OS->Toggle();
	}
}

// ARROWS: one page decides.
void AVEILPlayerController::ArrowUp()
{
	if (bGodMenuOpen) { GodMenuUp(); return; }
	if (bShowChart) { ChartMovePrev(); }
}

void AVEILPlayerController::ArrowDown()
{
	if (bGodMenuOpen) { GodMenuDown(); return; }
	if (bShowChart) { ChartMoveNext(); }
}

void AVEILPlayerController::ArrowLeft()
{
	if (bGodMenuOpen) { GodMenuLeft(); return; }
	if (bShowChart && ChartPage == 2) { OptionsAdjust(-1); }
}

void AVEILPlayerController::ArrowRight()
{
	if (bGodMenuOpen) { GodMenuRight(); return; }
	if (bShowChart && ChartPage == 2) { OptionsAdjust(1); }
}

// A AND X DO THE THING IN FRONT OF THE PLAYER.
// In a conversation they answer; outside one they travel and talk. Bound once, so
// a reply can never also fire a jump or a line of dialogue on the same press.
void AVEILPlayerController::PadBottom()
{
	if (const UOMNIQuestSystem* Quest = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
	{
		if (Quest->ChoiceOpen()) { ChooseReply(0); return; }
	}
	TravelOrAdvance();
}

void AVEILPlayerController::PadFace()
{
	if (const UOMNIQuestSystem* Quest = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
	{
		if (Quest->ChoiceOpen()) { ChooseReply(1); return; }
	}
	TalkToNearest();
}

// ONE D-PAD, ONE ACTION.
// Up on the pad used to be map-zoom, chart-previous and respawn at the same time,
// so a player reaching for the map could quietly kill and respawn the runner. The
// page on screen decides now, and with no page open the d-pad does nothing that
// can hurt: up and down are the chart and the map, left is the field manual,
// right switches the lead.
void AVEILPlayerController::PadUp()
{
	if (bPauseOpen) { return; }
	if (bGodMenuOpen) { GodMenuUp(); return; }
	if (bShowChart) { if (ChartPage == 2) { OptionsMove(-1); } else { MapZoomIn(); } return; }
	if (bShowFieldManual) { ToggleFieldManual(); return; }
	UE_LOG(LogVEIL, Log, TEXT("PHI PAD: up - nothing open; the chart is on VIEW, not here."));
}

void AVEILPlayerController::PadDown()
{
	if (bPauseOpen) { return; }
	if (bGodMenuOpen) { GodMenuDown(); return; }
	if (bShowChart) { if (ChartPage == 2) { OptionsMove(1); } else { MapZoomOut(); } return; }
	ToggleTips();
}

void AVEILPlayerController::PadLeft()
{
	if (bPauseOpen) { return; }
	if (bGodMenuOpen) { GodMenuLeft(); return; }
	if (bShowChart && ChartPage == 2) { OptionsAdjust(-1); return; }
	if (bShowChart) { return; }
	ToggleFieldManual();
}

void AVEILPlayerController::PadRight()
{
	if (bPauseOpen) { return; }
	if (bGodMenuOpen) { GodMenuRight(); return; }
	if (bShowChart && ChartPage == 2) { OptionsAdjust(1); return; }
	if (bShowChart) { return; }
	SwitchLead();
}

// ---------------------------------------------------------------------------
// CLICK TO TYPE TO ONE. A key you have to hold is not a conversation; a field you
// click and type into is. The HUD draws the bar from VEILTypeBar::ForScreen and
// this hit-tests the very same rectangle, so the click and the drawing agree.
// ---------------------------------------------------------------------------
void AVEILPlayerController::OnLeftClickType()
{
	float MX = 0.f, MY = 0.f;
	if (!GetMousePosition(MX, MY)) { return; }

	float ClipX = 1920.f, ClipY = 1080.f;
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
	{
		const FIntPoint Size = GEngine->GameViewport->Viewport->GetSizeXY();
		ClipX = (float)Size.X;
		ClipY = (float)Size.Y;
	}
	const VEILTypeBar::FZone Zone = VEILTypeBar::ForScreen(ClipX, ClipY);
	if (!Zone.Contains(MX, MY)) { return; }

	OpenTypeToONE();
}

void AVEILPlayerController::OpenTypeToONE()
{
	if (!GEngine || !GEngine->GameViewport || !GEngine->GameViewport->Viewport) { return; }
	if (bTypingToONE && TypeBox.IsValid())
	{
		FSlateApplication::Get().SetKeyboardFocus(TypeBox, EFocusCause::SetDirectly);
		return;
	}

	SAssignNew(TypeBox, SEditableTextBox)
		.HintText(FText::FromString(TEXT("Type to ONE, then press Enter")))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
		.ForegroundColor(FSlateColor(FLinearColor(1.f, 0.88f, 0.45f)))
		.BackgroundColor(FSlateColor(FLinearColor(0.035f, 0.045f, 0.075f, 0.95f)))
		.Padding(FMargin(10.f, 5.f))
		.SelectAllTextWhenFocused(true)
		.OnTextCommitted_UObject(this, &AVEILPlayerController::OnOneTextCommitted);

	const FIntPoint Size = GEngine->GameViewport->Viewport->GetSizeXY();
	const VEILTypeBar::FZone Zone = VEILTypeBar::ForScreen((float)Size.X, (float)Size.Y);
	SAssignNew(TypeContainer, SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, Zone.Y, 0.f, 0.f))
		.WidthOverride(Zone.W)
		[ TypeBox.ToSharedRef() ];

	GEngine->GameViewport->AddViewportWidgetContent(TypeContainer.ToSharedRef(), 100);

	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(TypeBox);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);
	bShowMouseCursor = true;
	bTypingToONE = true;
	UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: field open - click landed in the type bar."));
}

void AVEILPlayerController::CloseTypeToONE()
{
	if (TypeContainer.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(TypeContainer.ToSharedRef());
	}
	TypeContainer.Reset();
	TypeBox.Reset();
	bTypingToONE = false;
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void AVEILPlayerController::SubmitTypedLine(const FString& Line)
{
	const FString Clean = Line.TrimStartAndEnd();
	if (Clean.IsEmpty())
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: empty line, nothing sent."));
		return;
	}
	if (UWorld* W = GetWorld())
	{
		if (UPHIONE* One = W->GetSubsystem<UPHIONE>())
		{
			One->Say(Clean);
			UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: sent to ONE - %s (%d chars)"), *Clean, Clean.Len());
			return;
		}
	}
	UE_LOG(LogVEIL, Error, TEXT("PHI TYPE: ONE subsystem is missing, the line went nowhere."));
}

void AVEILPlayerController::OnOneTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		SubmitTypedLine(Text.ToString());
	}
	CloseTypeToONE();
	UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: field closed after %s."),
		CommitType == ETextCommit::OnEnter ? TEXT("Enter") : TEXT("focus loss"));
}

void AVEILPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AVEILPlayerController::TogglePauseMenu);
		// [F1] raises the one OS: face over engine, one surface for the whole game.
		InputComponent->BindKey(EKeys::F1, IE_Pressed, this, &AVEILPlayerController::ToggleOS);
		// Story input: E speaks, SPACE advances the line.
		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AVEILPlayerController::TalkToNearest);
		// A pad speaks and answers: X talks, A / X / Y pick replies 1 / 2 / 3
		// while a choice waits (guarded inside ChooseReply, so outside a
		// conversation these keys keep their own jobs).
		InputComponent->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Pressed, this, &AVEILPlayerController::PadReply3);
		// GODMODE page: [G] opens it, arrows walk it, SPACE (above) works the row.
		InputComponent->BindKey(EKeys::G, IE_Pressed, this, &AVEILPlayerController::ToggleGodMenu);
		// Two columns: left / right jump a column, up / down walk the rows.
		// The living map: wheel on a mouse, D-pad up/down on a pad.
		InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &AVEILPlayerController::MapZoomIn);
		InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &AVEILPlayerController::MapZoomOut);
		// CLICK TO TYPE. Left mouse over the type bar opens the field; everywhere else the
		// click is left to the world, which is why this tests only the bar's own rectangle.
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AVEILPlayerController::OnLeftClickType);
		InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AVEILPlayerController::TravelOrAdvance);
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AVEILPlayerController::TogglePauseMenu);
		InputComponent->BindKey(EKeys::Gamepad_Special_Right, IE_Pressed, this, &AVEILPlayerController::TogglePauseMenu);

		// EVERY ADVERTISED KEY IS BOUND, AND EVERY ONE HAS A PAD EQUIVALENT.
		// The HUD used to print [TAB], [R], [C] and [M] with nothing behind them.
		// Chart
		// Tab / VIEW walks the pages: the chart, then the living world.
		InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AVEILPlayerController::NextChartPage);
		InputComponent->BindKey(EKeys::Gamepad_Special_Left, IE_Pressed, this, &AVEILPlayerController::NextChartPage);
		// Archetypes: 1..9 and 0 stand the runner in any of the twelve. No lock.
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AVEILPlayerController::SelectArchetype0);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AVEILPlayerController::SelectArchetype1);
		InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AVEILPlayerController::SelectArchetype2);
		InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AVEILPlayerController::SelectArchetype3);
		InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AVEILPlayerController::SelectArchetype4);
		InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &AVEILPlayerController::SelectArchetype5);
		InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &AVEILPlayerController::SelectArchetype6);
		InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &AVEILPlayerController::SelectArchetype7);
		InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &AVEILPlayerController::SelectArchetype8);
		InputComponent->BindKey(EKeys::Zero, IE_Pressed, this, &AVEILPlayerController::SelectArchetype9);
		// Chart navigation + travel: arrows / d-pad move the highlight, Enter or A
		// carries the runner to it. The d-pad's other jobs (respawn, mute) step
		// aside while the chart is open - see their guards.
		// Options: the same d-pad / arrows, left and right to change a value.
		// O jumps straight to the options page from anywhere.
		InputComponent->BindKey(EKeys::O, IE_Pressed, this, &AVEILPlayerController::OpenOptions);
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AVEILPlayerController::ChartTravel);
		// Field manual: H (safe), the pad's left d-pad, and F1 handled in InputKey
		// because the engine owns F1 and must be intercepted first.
		InputComponent->BindKey(EKeys::H, IE_Pressed, this, &AVEILPlayerController::ToggleFieldManual);
		// F10: the debug strips. Off by default - normal play shows the game, not the
		// instrumentation. The Bible names F10 for exactly this.
		InputComponent->BindKey(EKeys::F10, IE_Pressed, this, &AVEILPlayerController::ToggleDebugStrips);
		// L: the log. Hidden by default, per the locked HUD spec.
		InputComponent->BindKey(EKeys::L, IE_Pressed, this, &AVEILPlayerController::ToggleLog);
		// Respawn
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AVEILPlayerController::RespawnRunner);
		// Switch lead
		InputComponent->BindKey(EKeys::C, IE_Pressed, this, &AVEILPlayerController::SwitchLead);
		// Narrator mute
		InputComponent->BindKey(EKeys::M, IE_Pressed, this, &AVEILPlayerController::ToggleMuteNarrator);
		// Hide tips. Not F2: the engine still answers F2 with "viewmode unlit".
		InputComponent->BindKey(EKeys::T, IE_Pressed, this, &AVEILPlayerController::ToggleTips);

		// ARROWS AND THE PAD'S A/X, ALSO ROUTED. Arrows carried three jobs each
		// (cheat page, chart, options) and A/X carried two (travel/talk and reply),
		// so a press could do two things at once. One dispatcher per key now.
		InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &AVEILPlayerController::ArrowUp);
		InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &AVEILPlayerController::ArrowDown);
		InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &AVEILPlayerController::ArrowLeft);
		InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &AVEILPlayerController::ArrowRight);
		InputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Pressed, this, &AVEILPlayerController::PadBottom);
		InputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed, this, &AVEILPlayerController::PadFace);

		// THE D-PAD, ROUTED. One press, one action, decided by the page in front of
		// the player - not by which of three bindings happened to pass its guard.
		InputComponent->BindKey(EKeys::Gamepad_DPad_Up, IE_Pressed, this, &AVEILPlayerController::PadUp);
		InputComponent->BindKey(EKeys::Gamepad_DPad_Down, IE_Pressed, this, &AVEILPlayerController::PadDown);
		InputComponent->BindKey(EKeys::Gamepad_DPad_Left, IE_Pressed, this, &AVEILPlayerController::PadLeft);
		InputComponent->BindKey(EKeys::Gamepad_DPad_Right, IE_Pressed, this, &AVEILPlayerController::PadRight);

		// Story verbs on the pad: X speaks, A advances (and jumps when nobody talks).
	}
}

bool AVEILPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	const FKey K = Params.Key;

	// F1-F5 are the ENGINE's debug viewmode keys and they fire even in a -game
	// session ("Set new viewmode: Wireframe" in the log when F1 was pressed for
	// help). They are claimed here, before the exec handler can see them: F1
	// opens the field manual, the rest do nothing at all.
	if (K == EKeys::F1)
	{
		if (Params.Event == IE_Pressed)
		{
			VEILInput::NoteKeyboard();
			bUsingGamepad = false;
			ToggleFieldManual();
		}
		return true;
	}
	if (K == EKeys::F2 || K == EKeys::F3 || K == EKeys::F4 || K == EKeys::F5)
	{
		return true;
	}
	if (K.IsGamepadKey())
	{
		VEILInput::NoteGamepad();
		bUsingGamepad = true;
	}
	else if (Params.Event == IE_Pressed || Params.Event == IE_Released)
	{
		VEILInput::NoteKeyboard();
		bUsingGamepad = false;
	}
	return Super::InputKey(Params);
}

void AVEILPlayerController::ToggleChart()
{
	bShowChart = !bShowChart;
	if (bShowChart)
	{
		// Open on where he stands, so the list starts from home.
		if (UVEILFastTravel* FT = GetWorld() ? GetWorld()->GetSubsystem<UVEILFastTravel>() : nullptr)
		{
			if (const APawn* P = GetPawn())
			{
				const int32 Here = FT->StandingAtIndex(P->GetActorLocation());
				if (Here != INDEX_NONE) { ChartSel = Here; }
			}
			ChartSel = FMath::Clamp(ChartSel, 0, FMath::Max(0, FT->Waypoints.Num() - 1));
			UE_LOG(LogVEIL, Log, TEXT("PHI CHART: open - %s."), *FT->Status());
			return;
		}
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI CHART: %s"), bShowChart ? TEXT("open") : TEXT("shut"));
}

void AVEILPlayerController::NextChartPage()
{
	// closed -> chart -> living world -> closed. Tab / VIEW walks it.
	if (!bShowChart)
	{
		bShowChart = true;
		ChartPage = 0;
	}
	else if (ChartPage == 0)
	{
		ChartPage = 1;
	}
	else if (ChartPage == 1)
	{
		ChartPage = 2;
		OptionsSel = 0;
	}
	else
	{
		bShowChart = false;
		ChartPage = 0;
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI PAGES: %s (page %d)"), bShowChart ? TEXT("open") : TEXT("shut"), ChartPage);
}

void AVEILPlayerController::SelectArchetype(int32 Index)
{
	// Context first: while a speaker is waiting on an answer, the number keys are
	// the replies, not the archetypes. Same keys, no new keys to learn, and the
	// HUD names them in the choice box.
	if (UVEILDialogue* Dlg = GetWorld() ? GetWorld()->GetSubsystem<UVEILDialogue>() : nullptr)
	{
		if (Dlg->IsChoosing())
		{
			Dlg->Choose(Index);
			return;
		}
	}

	const TArray<FVEILArchetype>& All = VEILSociety::Archetypes();
	if (!All.IsValidIndex(Index)) { return; }
	ActiveArchetype = Index;
	UE_LOG(LogVEIL, Log, TEXT("PHI ARCHETYPE: runner now stands in %s - %s. Nothing is locked."),
		*All[Index].Name, *All[Index].Power);
}

// ---------------------------------------------------------------------------
// PHI CHEATS: the GODMODE page.
// ---------------------------------------------------------------------------

void AVEILPlayerController::ChooseReply(int32 Index)
{
	UVEILDialogue* Dlg = GetWorld() ? GetWorld()->GetSubsystem<UVEILDialogue>() : nullptr;
	if (!Dlg || !Dlg->IsChoosing()) { return; }
	// Same handler the number keys use, so a pad answer and a key answer are the
	// same act in the world - not a second path with its own behaviour.
	SelectArchetype(Index);
	UE_LOG(LogVEIL, Log, TEXT("PHI PAD: reply %d chosen from the controller."), Index + 1);
}

void AVEILPlayerController::PadReply1() { ChooseReply(0); }   // A
void AVEILPlayerController::PadReply2() { ChooseReply(1); }   // X
void AVEILPlayerController::PadReply3() { ChooseReply(2); }   // Y

void AVEILPlayerController::ToggleGodMenu()
{
	bGodMenuOpen = !bGodMenuOpen;
	GodSel = FMath::Clamp(GodSel, 0, UPHICheats::RowCount - 1);
	UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: GODMODE page %s."), bGodMenuOpen ? TEXT("open") : TEXT("closed"));
}

void AVEILPlayerController::GodMenuMove(int32 Delta)
{
	GodSel = (GodSel + Delta + UPHICheats::RowCount) % UPHICheats::RowCount;
}

void AVEILPlayerController::GodMenuUp() { if (bGodMenuOpen) { GodMenuMove(-1); } }
void AVEILPlayerController::GodMenuDown() { if (bGodMenuOpen) { GodMenuMove(1); } }
void AVEILPlayerController::GodMenuLeft() { if (bGodMenuOpen) { GodMenuMove(-UPHICheats::RowsPerColumn); } }
void AVEILPlayerController::GodMenuRight() { if (bGodMenuOpen) { GodMenuMove(UPHICheats::RowsPerColumn); } }

void AVEILPlayerController::GodMenuActivate()
{
	if (UPHICheats* Cheats = GetWorld() ? GetWorld()->GetSubsystem<UPHICheats>() : nullptr)
	{
		Cheats->Activate(GodSel);
	}
}

// ---------------------------------------------------------------------------
// WHO OWNS THE MOUSE
// ---------------------------------------------------------------------------

bool AVEILPlayerController::UIWantsCursor() const
{
	// The type bar owns the pointer while it is open, or the cursor is hidden the
	// same frame it is needed and the player cannot click into the field at all.
	if (bTypingToONE) { return true; }
	// Only three pages in this game take the pointer: the title, the pause page
	// and the ending. The chart, the field manual and the GODMODE page are
	// keyboard / pad pages by design, so they never steal the mouse.
	const bool bMenuUp = MainMenuWidget && MainMenuWidget->IsInViewport();
	return bMenuUp || bPauseOpen || bWinScreenOpen;
}

void AVEILPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// THE MOUSE BELONGS TO THE LENS UNLESS A PAGE OWNS IT. A cursor left switched
	// on with nothing owning it is exactly the bug the player reports as "the
	// mouse is not working": GameAndUI keeps the pointer, so mouse-look dies -
	// while the wheel still reaches the game and zooms the minimap underneath it.
	// The mode is therefore re-asserted every frame from the pages that really
	// want a pointer, instead of being set once by whoever toggled last.
	const bool bWantCursor = UIWantsCursor();
	if (bWantCursor)
	{
		if (!bShowMouseCursor)
		{
			SetInputMode(FInputModeGameAndUI());
			bShowMouseCursor = true;
		}
		return;
	}

	if (bShowMouseCursor)
	{
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		UE_LOG(LogVEIL, Warning, TEXT("PHI INPUT: no page owned the cursor - the lens has the mouse again, look restored."));
	}
}

void AVEILPlayerController::MapZoomIn()
{
	// THE WHEEL ZOOMS THE MAP ONLY WHILE THE MAP IS OPEN. It was bound to the map
	// AND to the camera ring on the pawn, so a player who scrolled to look around
	// was silently zooming a map he could not see - the "stuck in zoom mode"
	// report. Context decides: chart open, the wheel is the map's; otherwise the
	// wheel is the camera ring's and the map does not move at all.
	if (!bShowChart) { return; }
	if (UVEILMapCapture* Map = GetWorld() ? GetWorld()->GetSubsystem<UVEILMapCapture>() : nullptr) { Map->ZoomIn(); }
}

void AVEILPlayerController::MapZoomOut()
{
	if (!bShowChart) { return; }
	if (UVEILMapCapture* Map = GetWorld() ? GetWorld()->GetSubsystem<UVEILMapCapture>() : nullptr) { Map->ZoomOut(); }
}

void AVEILPlayerController::OptionsDown() { OptionsAdjust(-1); }
void AVEILPlayerController::OptionsUp() { OptionsAdjust(1); }

void AVEILPlayerController::OpenOptions()
{
	bShowChart = true;
	ChartPage = 2;
	OptionsSel = 0;
}

void AVEILPlayerController::OptionsMove(int32 Delta)
{
	const int32 Rows = 8;
	OptionsSel = ((OptionsSel + Delta) % Rows + Rows) % Rows;
}

void AVEILPlayerController::OptionsAdjust(int32 Delta)
{
	// OptionsAdjust behaves: nothing to adjust unless the options page is the page.
	if (!bShowChart || ChartPage != 2) { return; }
	// The arrows and the d-pad only mean "change this" while the page is open.
	if (!bShowChart || ChartPage != 2) { return; }
	UVEILPerf* Perf = GetWorld() ? GetWorld()->GetSubsystem<UVEILPerf>() : nullptr;
	if (!Perf) { return; }

	switch (OptionsSel)
	{
	case 0: // quality preset
	{
		int32 Level = Perf->GetPreset();
		if (Level < 0) { Level = 2; }
		Level = FMath::Clamp(Level + Delta, 0, 3);
		Perf->SetPreset(Level);
		break;
	}
	case 1: // resolution scale, on the phi grid
		Perf->SetAdaptive(false);
		Perf->StepResolution(Delta);
		break;
	case 2: // frame cap
	{
		static const int32 Caps[] = { 30, 60, 72, 120, 144, 0 };
		int32 Index = 1;
		for (int32 i = 0; i < UE_ARRAY_COUNT(Caps); ++i)
		{
			if (Caps[i] == Perf->GetFrameCap()) { Index = i; break; }
		}
		Index = FMath::Clamp(Index + Delta, 0, (int32)UE_ARRAY_COUNT(Caps) - 1);
		Perf->SetFrameCap(Caps[Index]);
		break;
	}
	case 3: Perf->SetVsync(!Perf->GetVsync()); break;
	case 4: Perf->SetAdaptive(!Perf->IsAdaptive()); break;
	case 5: Perf->SetPreset(2); Perf->SetFrameCap(60); Perf->SetVsync(false); Perf->SetAdaptive(true); break; // recommended
	case 6: Perf->LogReport(); break;
	case 7: bShowChart = false; ChartPage = 0; break; // back
	default: break;
	}
}

void AVEILPlayerController::OptionsActivate()
{
	// Enter / A on the options page: same as one step right, and Back closes.
	if (OptionsSel == 7) { bShowChart = false; ChartPage = 0; return; }
	OptionsAdjust(1);
}

void AVEILPlayerController::SelectArchetype0() { SelectArchetype(0); }
void AVEILPlayerController::SelectArchetype1() { SelectArchetype(1); }
void AVEILPlayerController::SelectArchetype2() { SelectArchetype(2); }
void AVEILPlayerController::SelectArchetype3() { SelectArchetype(3); }
void AVEILPlayerController::SelectArchetype4() { SelectArchetype(4); }
void AVEILPlayerController::SelectArchetype5() { SelectArchetype(5); }
void AVEILPlayerController::SelectArchetype6() { SelectArchetype(6); }
void AVEILPlayerController::SelectArchetype7() { SelectArchetype(7); }
void AVEILPlayerController::SelectArchetype8() { SelectArchetype(8); }
void AVEILPlayerController::SelectArchetype9() { SelectArchetype(9); }

void AVEILPlayerController::ChartMovePrev() { ChartMove(-1); }
void AVEILPlayerController::ChartMoveNext() { ChartMove(1); }

void AVEILPlayerController::ChartMove(int32 Delta)
{
	if (!bShowChart) { return; }
	if (ChartPage == 2) { OptionsMove(Delta); return; }
	UVEILFastTravel* FT = GetWorld() ? GetWorld()->GetSubsystem<UVEILFastTravel>() : nullptr;
	if (!FT || FT->Waypoints.Num() == 0) { return; }
	const int32 N = FT->Waypoints.Num();
	ChartSel = ((ChartSel + Delta) % N + N) % N;
	const FVEILWaypoint& W = FT->Waypoints[ChartSel];
	UE_LOG(LogVEIL, Log, TEXT("PHI CHART: highlighted %d/%d %s (%s)"),
		ChartSel + 1, N, *W.Name, W.bLit ? TEXT("lit") : TEXT("unlit"));
}

void AVEILPlayerController::ChartTravel()
{
	if (!bShowChart) { return; }
	if (ChartPage != 0) { return; }
	UVEILFastTravel* FT = GetWorld() ? GetWorld()->GetSubsystem<UVEILFastTravel>() : nullptr;
	APawn* P = GetPawn();
	if (FT && P && FT->TravelTo(ChartSel, P))
	{
		bShowChart = false;
		// Travel moves through space; it also collapses the field onto the
		// farthest future, so a journey can land him later in the day.
		if (UVEILTime* Clock = GetWorld() ? GetWorld()->GetSubsystem<UVEILTime>() : nullptr)
		{
			Clock->Observe(TEXT("travel"));
		}
	}
}

void AVEILPlayerController::TravelOrAdvance()
{
	// SPACE JUMPS, TOO. Travel used to fire on the same key that jumps and on the
	// pad's A, so a hop across a plaza could carry the runner across the district.
	// Travel only means something while the chart is up or a line is waiting.
	{
		const UVEILDialogue* Dlg = GetWorld() ? GetWorld()->GetSubsystem<UVEILDialogue>() : nullptr;
		const bool bTalking = Dlg && Dlg->IsActive();
		if (!bShowChart && !bTalking) { return; }
	}
	// While the GODMODE page is up, this button is the switch under the cursor -
	// never a beacon jump behind the menu.
	if (bGodMenuOpen) { GodMenuActivate(); return; }

	// The button means the nearest thing: a line to advance, else the thing the
	// open page is asking for, else a beacon to travel to.
	if (UVEILDialogue* Dlg = GetWorld() ? GetWorld()->GetSubsystem<UVEILDialogue>() : nullptr)
	{
		// A waiting choice owns the button: advancing past it would throw the
		// answer away. The replies are A / X / Y at that moment (PadReply1..3).
		if (Dlg->IsChoosing()) { return; }
		if (Dlg->IsActive()) { Dlg->Advance(); return; }
	}
	if (bShowChart && ChartPage == 2) { OptionsActivate(); return; }
	ChartTravel();
}

void AVEILPlayerController::ToggleLog()
{
	bShowLog = !bShowLog;
	UE_LOG(LogVEIL, Log, TEXT("PHI LOG: %s"), bShowLog ? TEXT("shown") : TEXT("hidden"));
}

void AVEILPlayerController::ToggleDebugStrips()
{
	bShowDebug = !bShowDebug;
	UE_LOG(LogVEIL, Log, TEXT("PHI DEBUG STRIPS: %s"), bShowDebug ? TEXT("shown (F10)") : TEXT("hidden"));
}

void AVEILPlayerController::ToggleFieldManual()
{
	// While a page is open the d-pad belongs to that page.
	if (bShowChart) { return; }
	bShowFieldManual = !bShowFieldManual;
	UE_LOG(LogVEIL, Log, TEXT("PHI FIELD MANUAL PAGE: %s"), bShowFieldManual ? TEXT("open") : TEXT("shut"));
}

void AVEILPlayerController::ToggleMuteNarrator()
{
	if (bShowChart) { return; }
	FPHIVoice::bMuted = !FPHIVoice::bMuted;
	UE_LOG(LogVEIL, Log, TEXT("PHI NARRATOR: %s"), FPHIVoice::bMuted ? TEXT("muted") : TEXT("audible"));
}

void AVEILPlayerController::SwitchLead()
{
	if (bShowChart) { return; }
	if (UWorld* W = GetWorld())
	{
		if (UPHICompanionParty* Party = W->GetSubsystem<UPHICompanionParty>())
		{
			Party->CyclePlayable();
			UE_LOG(LogVEIL, Log, TEXT("PHI PARTY: lead is now %s."), *Party->LeadName());
		}
	}
}

void AVEILPlayerController::ToggleTips()
{
	if (UWorld* W = GetWorld())
	{
		if (UVEILTutorial* T = W->GetSubsystem<UVEILTutorial>())
		{
			if (T->IsHidden()) { T->Show(); } else { T->Hide(); }
			UE_LOG(LogVEIL, Log, TEXT("PHI FIELD MANUAL: tips %s."), T->IsHidden() ? TEXT("hidden") : TEXT("shown"));
		}
	}
}

void AVEILPlayerController::TogglePauseMenu()
{
	// Escape over the cheat page closes the page, it does not stack a pause menu
	// on top of it.
	if (bGodMenuOpen && !bPauseOpen) { bGodMenuOpen = false; return; }

	bPauseOpen = !bPauseOpen;
	SetPause(bPauseOpen);

	if (bPauseOpen)
	{
		if (PauseWidgetClass && !PauseWidget)
		{
			PauseWidget = CreateWidget<UUserWidget>(this, PauseWidgetClass);
		}
		if (PauseWidget) PauseWidget->AddToViewport(10);
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = true;
	}
	else
	{
		if (PauseWidget) PauseWidget->RemoveFromParent();
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}

void AVEILPlayerController::ShowWinScreen(float Gold, int32 Level, float TimeSeconds)
{
	UE_LOG(LogTemp, Display, TEXT("PHI WIN SCREEN: gold=%.0f level=%d time=%.1f"), Gold, Level, TimeSeconds);
	bWinScreenOpen = true;
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;
}

void AVEILPlayerController::CloseWinScreen()
{
	bWinScreenOpen = false;
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void AVEILPlayerController::RespawnRunner()
{
	// While the chart is open the d-pad belongs to the destination list.
	if (bShowChart) { return; }
	APawn* OwnedPawn = GetPawn();
	if (!OwnedPawn)
	{
		return;
	}
	OwnedPawn->SetActorLocation(FVector(0.f, -1500.f, 100.f));
	UE_LOG(LogTemp, Display, TEXT("PHI RESPAWN RUNNER at 0,0,100"));
}

void AVEILGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	static float GmProofClock = 0.f;
	GmProofClock += DeltaSeconds;
	if (GmProofClock >= 5.f)
	{
		GmProofClock = 0.f;
		UE_LOG(LogTemp, Display, TEXT("PHI GM TICKPROOF: worldTime=%.1f paused=%d"),
			GetWorld()->GetTimeSeconds(), UGameplayStatics::IsGamePaused(GetWorld()) ? 1 : 0);
	}
}

void AVEILPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// ONE CAMERA, HELD WHOLE. The rig owns its own pitch (golden rest, phi
	// envelope) and the arm never re-derives it, so the controller no longer
	// pins control pitch - that pinning is what made the camera unable to look
	// up or down at all. The rig is stepped here, on the loop proven to run.
	if (AVEILCharacter* Runner = Cast<AVEILCharacter>(GetPawn()))
	{
		Runner->HoldGoldenCamera(DeltaSeconds);
	}

	// PHI CAMERA PROBE: four seconds of real tick, then say exactly where the
	// lens is, how far it is from the runner, and the first thing it faces. The
	// world clock here reads 0.00 and world timers never fire, so this rides the
	// one loop that is proven to run - the controller's own tick.
	{
		static float CameraProbeClock = 0.f;
		static bool bCameraProbeDone = false;
		if (!bCameraProbeDone)
		{
			CameraProbeClock += DeltaSeconds;
			if (CameraProbeClock >= 4.f)
			{
				bCameraProbeDone = true;
				if (AVEILCharacter* PhiRunner = Cast<AVEILCharacter>(GetPawn()))
				{
					if (const UCameraComponent* Lens = PhiRunner->FindComponentByClass<UCameraComponent>())
					{
						const FVector LensLocation = Lens->GetComponentLocation();
						const FVector LensForward = Lens->GetForwardVector();
						UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA PROBE: runner %s lens %s fwd %s, %.0f cm from the body."),
							*PhiRunner->GetActorLocation().ToCompactString(), *LensLocation.ToCompactString(),
							*LensForward.ToCompactString(),
							FVector::Dist(LensLocation, PhiRunner->GetActorLocation()));
						FHitResult LensHit;
						FCollisionQueryParams ProbeParams(SCENE_QUERY_STAT(PhiCameraProbe), false, PhiRunner);
						if (GetWorld()->LineTraceSingleByChannel(LensHit, LensLocation,
							LensLocation + LensForward * 2000.f, ECC_Visibility, ProbeParams) && LensHit.GetActor())
						{
							UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA PROBE: first thing in the lens: %s (%s) at %.0f cm."),
								*LensHit.GetActor()->GetName(), *LensHit.GetActor()->GetClass()->GetName(),
								LensHit.Distance);
						}
						else
						{
							UE_LOG(LogVEIL, Log, TEXT("PHI CAMERA PROBE: nothing within 2000 cm of the lens."));
						}
					}
				}
			}
		}
	}

	static float PcProofClock = 0.f;
	PcProofClock += DeltaSeconds;
	if (PcProofClock >= 5.f)
	{
		PcProofClock = 0.f;
		AVEILCharacter* PhiPawn = Cast<AVEILCharacter>(GetPawn());
		UE_LOG(LogTemp, Display, TEXT("PHI PC TICKPROOF: world=%s sameWorld=%d worldTime=%.2f worldDelta=%.4f dilation=%.3f pcTick=%d pcCanTick=%d pawn=%s pawnTick=%d pawnCanTick=%d pawnBegun=%d pawnTickEntries=%d timerActive=%d worldType=%d gameWorld=%d worldBegun=%d timerRemaining=%.2f"),
			GetWorld() ? *GetWorld()->GetName() : TEXT("none"),
			(GetPawn() && GetPawn()->GetWorld() == GetWorld()) ? 1 : 0,
			GetWorld()->GetTimeSeconds(), GetWorld()->GetDeltaSeconds(),
			UGameplayStatics::GetGlobalTimeDilation(this),
			IsActorTickEnabled() ? 1 : 0, PrimaryActorTick.bCanEverTick ? 1 : 0,
			GetPawn() ? *GetPawn()->GetName() : TEXT("none"),
			(GetPawn() && GetPawn()->IsActorTickEnabled()) ? 1 : 0,
			(GetPawn() && GetPawn()->PrimaryActorTick.bCanEverTick) ? 1 : 0,
			(GetPawn() && GetPawn()->HasActorBegunPlay()) ? 1 : 0,
			PhiPawn ? PhiPawn->PhiTickEntryCount : -1,
			GetWorldTimerManager().IsTimerActive(GPhiTimerHandle) ? 1 : 0,
			(int32)GetWorld()->WorldType, GetWorld()->IsGameWorld() ? 1 : 0,
			GetWorld()->HasBegunPlay() ? 1 : 0,
			GetWorldTimerManager().GetTimerRemaining(GPhiTimerHandle));
	}
}

void AVEILPlayerController::PhiOnTimerCheck()
{
	UE_LOG(LogTemp, Display, TEXT("PHI TIMER CHECK FIRED: worldTime=%.2f"), GetWorld()->GetTimeSeconds());
}
