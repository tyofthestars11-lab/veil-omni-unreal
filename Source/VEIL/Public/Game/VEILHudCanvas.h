// VEIL: THE LAST SIGNAL - canvas HUD. UMG does not composite in this
// -game session (proven: built, in viewport, ticking, zero pixels), so the
// readable loop display rides the AHUD canvas pass like stat text does.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VEILHudCanvas.generated.h"

UCLASS()
class VEIL_API AVEILHudCanvas : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	/** One legend, two places: a compact row under the frame's edge in play, and
	 *  the full key on the chart page. Both read the SAME marker colours the map
	 *  draws with, so a key can never disagree with the blips it explains. */
	void DrawMapLegend(class UCanvas* C, class UFont* Font, float X, float Y, bool bWide);
};
