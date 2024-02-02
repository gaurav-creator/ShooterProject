
#include "Character/Animation/ALSPlayerCameraBehavior.h"


#include "Character/ALSBaseCharacter.h"

void UALSPlayerCameraBehavior::SetRotationMode(EALSRotationMode RotationMode)
{
	bVelocityDirection = RotationMode == EALSRotationMode::VelocityDirection;
	bLookingDirection = RotationMode == EALSRotationMode::LookingDirection;
	bAiming = RotationMode == EALSRotationMode::Aiming;
}
