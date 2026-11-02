// PHI self update: the game audits itself at boot and rescues itself live.
// Boot audit verifies voices, effects, settings and the district's key
// actors, repairs what can be repaired, and says what it did. Live rescue
// catches the Runner below the world and walks him home. No errors are
// left to rot: every fault becomes a log line and, where possible, a fix.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PHISelfUpdate.generated.h"

UCLASS()
class VEIL_API UPHISelfUpdate : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Boot audit. Runs once from the GameMode at play start. */
	void BootAudit();

	/** Live rescue, driven from the GameMode tick. */
	void TickWorld(float DeltaSeconds);

private:
	bool bAudited = false;
	float RescueTimer = 0.f;
	int32 AuditAsset(const TCHAR* Path) const;
};
