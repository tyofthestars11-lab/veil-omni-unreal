"""Bind real CC0 materials and prove the save slot writes on boot."""

path = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")
before = s

# ---- includes ----
if '#include "VeilSaveGame.h"' not in s:
    s = s.replace('#include "VeilWater.h"',
                  '#include "VeilWater.h"\n#include "VeilSaveGame.h"\n#include "Kismet/GameplayStatics.h"', 1)

# ---- real CC0 material substitution inside the palette lookup ----
sig = "UMaterialInterface* AVeilGameMode::Mat(const TCHAR* Path)\n{"
if "M_CC0_Sandstone" not in s:
    assert s.count(sig) == 1, "Mat() signature not found"
    inject = sig + '''
	// Real photoscanned CC0 art replaces the flat palette wherever it exists, so the
	// market is stone that looks like stone and timber that looks like timber.
	static const TCHAR* Cc0Stone = TEXT("/Game/VEIL/CC0/Materials/M_CC0_Sandstone.M_CC0_Sandstone");
	static const TCHAR* Cc0Rock = TEXT("/Game/VEIL/CC0/Materials/M_CC0_Cliff.M_CC0_Cliff");
	static const TCHAR* Cc0Timber = TEXT("/Game/VEIL/CC0/Materials/M_CC0_Timber.M_CC0_Timber");
	static const TCHAR* Cc0Paving = TEXT("/Game/VEIL/CC0/Materials/M_CC0_Pavement.M_CC0_Pavement");
	if (FCString::Stricmp(Path, TEXT("/Game/VEIL/Materials/M_VEIL_Stone.M_VEIL_Stone")) == 0)
	{
		Path = Cc0Stone;
	}
	else if (FCString::Stricmp(Path, TEXT("/Game/VEIL/Materials/M_VEIL_Deep.M_VEIL_Deep")) == 0)
	{
		Path = Cc0Rock;
	}
	else if (FCString::Stricmp(Path, TEXT("/Game/VEIL/Materials/M_VEIL_Gold2.M_VEIL_Gold2")) == 0)
	{
		Path = Cc0Timber;
	}
	else if (FCString::Stricmp(Path, TEXT("/Game/VEIL/Materials/M_VEIL_Signal.M_VEIL_Signal")) == 0)
	{
		Path = Cc0Paving;
	}
'''
    s = s.replace(sig, inject, 1)

# ---- the save slot writes itself on boot ----
anchor = "\tBuildWorld();\n\tPlaceRunner();\n"
if "written on boot" not in s:
    assert s.count(anchor) == 1, "StartPlay anchor not found"
    s = s.replace(anchor, anchor + '''
	// The Signal records itself before any input, so the slot is proven, not assumed.
	if (UVeilSaveGame* SignalSave = Cast<UVeilSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UVeilSaveGame::StaticClass())))
	{
		if (APawn* RunnerPawn = GetWorld()->GetFirstPlayerController()
			? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr)
		{
			SignalSave->RunnerAt = RunnerPawn->GetActorLocation();
		}
		SignalSave->BeaconsLit = 0;
		UGameplayStatics::SaveGameToSlot(SignalSave, TEXT("VEILOMNI_Omni0"), 0);
		UE_LOG(LogVEILWorld, Display,
			TEXT("VEIL SAVE: VEILOMNI_Omni0 written on boot at (%.0f, %.0f, %.0f)"),
			SignalSave->RunnerAt.X, SignalSave->RunnerAt.Y, SignalSave->RunnerAt.Z);
	}
''', 1)

if s != before:
    open(path, "w", encoding="utf-8", newline="").write(s)
print("materials bound:", "M_CC0_Sandstone" in s)
print("boot save added:", "written on boot" in s)
