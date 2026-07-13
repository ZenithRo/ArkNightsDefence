#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TDMusicManager.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS(Blueprintable, BlueprintType)
class ARKNIGHTSDEFENCE_API UTDMusicManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Music")
	void PlayMusic(USoundBase* Music, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Music")
	void StopMusic(float FadeOutDuration = 0.5f);

	UFUNCTION(BlueprintPure, Category = "Music")
	float GetPlaybackProgress() const;

protected:
	UPROPERTY()
	TObjectPtr<USoundBase> CurrentMusic;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComp;

	double PlayStartTime = 0.0;

	void InternalPlay(USoundBase* Music, float Volume);
};
