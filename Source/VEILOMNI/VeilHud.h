#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VeilHud.generated.h"

class AVeilPlayerController;

/**
 * The gold ribbon. Not black panels, not plain rectangles.
 *
 * Ribbon: a curved phi-tapered gold band across the top, carrying VIT in red,
 * STM in green and the LV diamond at its centre.
 * Minimap: the Bible's round gold ring, N/E/S/W ticks, compass needle and the pause
 * button inside the ring - and under it a real map, drawn from the actual static
 * meshes of the level, north-up, centred on the Runner. Nothing on it is invented.
 * Quest: an angled gold banner. Dialogue: [E] only, with the speaker named.
 */
UCLASS()
class VEILOMNI_API AVeilHud : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	virtual void NotifyHitBoxClick(FName BoxName) override;

private:
	void Ribbon(float W, float H, float Fill);
	void Vitals(AVeilPlayerController* PC, float W, float H);
	void Minimap(AVeilPlayerController* PC, float W, float H);
	void QuestBanner(AVeilPlayerController* PC, float W, float H);
	void Abilities(AVeilPlayerController* PC, float W, float H);
	void Prompt(AVeilPlayerController* PC, float W, float H);
	void Dialogue(AVeilPlayerController* PC, float W, float H);

	void CollectMapGeometry();
	void Disc(float CX, float CY, float R, const FLinearColor& C);

	void GoldLine(float X1, float Y1, float X2, float Y2, float T, const FLinearColor& C);
	void Diamond(float CX, float CY, float R, const FLinearColor& C, float T);
	void Bar(float X, float Y, float W, float H, float Frac, const FLinearColor& Fill);

	/** Real level geometry, refreshed on a timer rather than rebuilt every frame. */
	TArray<FVector2D> MapBlips;
	TArray<float> MapSizes;
	TArray<float> MapHeights;
	float MapStamp = -1.f;
};
