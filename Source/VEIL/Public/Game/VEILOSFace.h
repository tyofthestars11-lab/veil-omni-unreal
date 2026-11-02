#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;

/**
 * The face of the one OS: screen, voice, map, controls - and under them the
 * engine it is reading (rungs, addresses, skills, automations, ledger).
 *
 * Drawn in its own pass so the world HUD stays the world HUD and the OS is a
 * surface you raise over it, not another row of text in it.
 */
namespace VEILOSFace
{
	void Draw(AHUD* H, UCanvas* C);
}
