#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;

/**
 * The parts of the reference mockup the world HUD was missing:
 * the gold sweep behind the plate, the quest banner bottom-left, the diamond
 * hotbar bottom-centre, and the COMBAT MODE pill bottom-right.
 *
 * Drawn in its own pass so the instrument in VEILHudCanvas stays the
 * instrument, and this layer only supplies the silhouette the painting asks for.
 */
namespace VEILHudMockup
{
	void Draw(AHUD* H, UCanvas* C);
}
