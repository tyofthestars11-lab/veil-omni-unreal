#pragma once

#include "CoreMinimal.h"

/**
 * THE TYPE BAR - one geometry, two readers.
 *
 * ONE is talked to by clicking this bar and typing, not only by holding B. The HUD draws the
 * bar and the player controller hit-tests the click, so both read the same rectangle from
 * here: a click can never land on a bar that is not drawn, or miss one that is.
 *
 * Width is the brief's one-line figure (480 px), height is one text row, and it sits above
 * the ONE state line and clear of the bottom action band so it owns its own strip.
 */
namespace VEILTypeBar
{
	struct FZone
	{
		float X = 0.f;
		float Y = 0.f;
		float W = 480.f;
		float H = 30.f;

		bool Contains(float PX, float PY) const
		{
			return PX >= X && PX <= X + W && PY >= Y && PY <= Y + H;
		}
	};

	inline FZone ForScreen(float ClipX, float ClipY)
	{
		FZone Z;
		Z.W = FMath::Min(480.f, FMath::Max(240.f, ClipX - 80.f));
		Z.H = 30.f;
		Z.X = (ClipX - Z.W) * 0.5f;
		Z.Y = ClipY - 212.f;   // above the ONE line at ClipY-170, clear of the action band
		return Z;
	}
}
