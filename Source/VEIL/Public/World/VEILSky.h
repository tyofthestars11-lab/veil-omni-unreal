#pragma once

#include "CoreMinimal.h"

class UWorld;

/**
 * THE LIGHT THE CITY IS DRAWN IN.
 *
 * A district of correct stone under white light still reads as grey blocks. The vision is a
 * golden city: a low warm sun, a sky light that takes its colour from that sun, and fog that
 * gives the valley depth instead of a hard horizon. This dresses what already exists rather
 * than adding a second sun, so the map is never double-lit.
 */
namespace VEILSky
{
	void Dress(UWorld* World);
}
