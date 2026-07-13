#include "Audio/TDMusicManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UTDMusicManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTDMusicManager::Deinitialize()
{
	StopMusic(0.0f);

	if (AudioComp && AudioComp->IsRegistered())
	{
		AudioComp->UnregisterComponent();
	}
	Super::Deinitialize();
}

void UTDMusicManager::PlayMusic(USoundBase* Music, float Volume)
{
	if (!Music) return;

	InternalPlay(Music, Volume);
}

void UTDMusicManager::StopMusic(float FadeOutDuration)
{
	if (!AudioComp) return;

	if (FadeOutDuration > 0.0f && AudioComp->IsPlaying())
	{
		AudioComp->FadeOut(FadeOutDuration, 0.0f);
	}
	else
	{
		AudioComp->Stop();
	}
	CurrentMusic = nullptr;
}

float UTDMusicManager::GetPlaybackProgress() const
{
	if (!AudioComp || !CurrentMusic) return 0.0f;

	float Duration = CurrentMusic->GetDuration();
	if (Duration <= 0.0f) return 0.0f;

	float Elapsed = static_cast<float>(FPlatformTime::Seconds() - PlayStartTime);
	return FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f);
}

void UTDMusicManager::InternalPlay(USoundBase* Music, float Volume)
{
	if (!Music) return;

	CurrentMusic = Music;
	PlayStartTime = FPlatformTime::Seconds();

	if (!AudioComp)
	{
		AudioComp = NewObject<UAudioComponent>(this);
		AudioComp->bAutoDestroy = false;
		AudioComp->bAutoActivate = false;
	}

	if (!AudioComp->IsRegistered() || !AudioComp->GetWorld())
	{
		if (UWorld* World = GetWorld())
		{
			AudioComp->RegisterComponentWithWorld(World);
		}
	}

	if (AudioComp->IsRegistered())
	{
		AudioComp->Stop();
		AudioComp->SetSound(Music);
		AudioComp->SetVolumeMultiplier(Volume);
		AudioComp->Play();
	}
	else
	{
		UGameplayStatics::PlaySound2D(this, Music, Volume);
	}
}
