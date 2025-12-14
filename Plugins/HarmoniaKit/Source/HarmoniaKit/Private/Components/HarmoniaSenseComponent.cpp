// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaSenseComponent.h"
#include "Components/HarmoniaSenseInteractableComponent.h"
#include "Components/HarmoniaSenseInteractionComponent.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "HarmoniaDataTableBFL.h"
#include "SenseReceiverComponent.h"
#include "SenseManager.h"
#include "Sensors/SensorBase.h"
#include "Sensors/ActiveSensor.h"
#include "Sensors/PassiveSensor.h"
#include "SenseSysHelpers.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "DrawDebugHelpers.h"

UHarmoniaSenseComponent::UHarmoniaSenseComponent()
{
#if WITH_EDITOR || UE_BUILD_DEBUG
	PrimaryComponentTick.bCanEverTick = true;  // Enable for debug draw
	PrimaryComponentTick.bStartWithTickEnabled = true;
#else
	PrimaryComponentTick.bCanEverTick = false;
#endif
	SetIsReplicatedByDefault(true);
}

void UHarmoniaSenseComponent::BeginPlay()
{
	Super::BeginPlay();

	// Auto-find ASC if not set
	if (!OwnerAbilitySystem)
	{
		OwnerAbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	}

	// Initialize SenseSystem components on owner
	InitializeOwnerSenseComponents();

	// Initialize sensors from DataTable
	InitializeSensorsFromDataTable();
}

void UHarmoniaSenseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAllSensors();
	Super::EndPlay(EndPlayReason);
}

void UHarmoniaSenseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if WITH_EDITOR || UE_BUILD_DEBUG
	// Draw debug for all active sensors based on their settings
	UDataTable* SenseConfigDT = UHarmoniaDataTableBFL::GetSenseConfigDataTable();
	if (!SenseConfigDT)
	{
		return;
	}

	for (const auto& Pair : ActiveSensors)
	{
		if (USensorBase* Sensor = Pair.Value)
		{
			// Only draw if sensor has bDrawSensor enabled
			if (!Sensor->bDrawSensor)
			{
				continue;
			}

			// Get debug config from DataTable by sensor tag
			TArray<FName> RowNames = SenseConfigDT->GetRowNames();
			for (const FName& RowName : RowNames)
			{
				FHarmoniaSenseConfigData* ConfigPtr = SenseConfigDT->FindRow<FHarmoniaSenseConfigData>(RowName, TEXT("TickComponent"));
				if (ConfigPtr && ConfigPtr->bEnableDebugDraw && ConfigPtr->SensorClass)
				{
					USensorBase* ConfigCDO = ConfigPtr->SensorClass->GetDefaultObject<USensorBase>();
					if (ConfigCDO && ConfigCDO->SensorTag == Sensor->SensorTag)
					{
						const FSenseSysDebugDraw& DebugConfig = ConfigPtr->DebugDrawConfig;
						Sensor->DrawDebugSensor(
							DebugConfig.Sensor_DebugTest,
							DebugConfig.Sensor_DebugCurrentSensed,
							DebugConfig.Sensor_DebugLostSensed,
							DebugConfig.Sensor_DebugBestSensed,
							DebugConfig.SenseSys_DebugAge,
							0.0f  // Duration (0 = single frame)
						);
						break;
					}
				}
			}
		}
	}
#endif
}

// ============================================================================
// Sensor Management
// ============================================================================


USensorBase* UHarmoniaSenseComponent::AddSensor(FName SenseConfigRowName)
{
	if (SenseConfigRowName.IsNone())
	{
		return nullptr;
	}

	// Get config from DataTable
	FHarmoniaSenseConfigData Config = GetSenseConfigByRowName(SenseConfigRowName);
	if (!Config.SensorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SENSOR] AddSensor: SensorClass not set for '%s'"), *SenseConfigRowName.ToString());
		return nullptr;
	}

	// Get SensorTag from the Sensor BP class CDO
	USensorBase* SensorCDO = Config.SensorClass->GetDefaultObject<USensorBase>();
	if (!SensorCDO)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SENSOR] AddSensor: Failed to get Sensor CDO for '%s'"), *SenseConfigRowName.ToString());
		return nullptr;
	}
	FName SensorTag = SensorCDO->SensorTag;

	if (ActiveSensors.Contains(SensorTag))
	{
		return ActiveSensors[SensorTag];
	}

	// Get the SenseReceiver from OwnerInteraction
	USenseReceiverComponent* Receiver = OwnerInteraction;
	if (!Receiver)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SENSOR] AddSensor: OwnerInteraction (SenseReceiver) is null"));
		return nullptr;
	}

	// Get sensor type and thread type from CDO
	ESensorType SensorType = SensorCDO->SensorType;
	ESensorThreadType ThreadType = SensorCDO->SensorThreadType;

	// Fallback: determine sensor type based on class hierarchy if CDO has None
	if (SensorType == ESensorType::None)
	{
		SensorType = Config.SensorClass->IsChildOf(UPassiveSensor::StaticClass()) ? ESensorType::Passive : ESensorType::Active;
	}

	// Create sensor using SenseReceiver's CreateNewSensor API
	// All settings come from the Sensor BP CDO
	ESuccessState SuccessState = ESuccessState::Failed;
	USensorBase* NewSensor = Receiver->CreateNewSensor(
		Config.SensorClass,
		SensorType,
		SensorTag,
		ThreadType,
		true,  // Enable immediately
		SuccessState
	);

	if (SuccessState == ESuccessState::Success && NewSensor)
	{
		ActiveSensors.Add(SensorTag, NewSensor);
		
		// Also register directly to OwnerInteraction's arrays (workaround for CreateNewSensor bug)
		if (OwnerInteraction)
		{
			OwnerInteraction->RegisterSensorDirectly(NewSensor);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SENSOR] AddSensor: Failed to create sensor '%s'"), *SensorTag.ToString());
	}

	return NewSensor;
}

bool UHarmoniaSenseComponent::RemoveSensor(FName SensorTag)
{
	if (!ActiveSensors.Contains(SensorTag))
	{
		return false;
	}

	USensorBase* Sensor = ActiveSensors[SensorTag];
	
	if (Sensor && !Sensor->IsSensorTaskWorkDone())
	{
		// Brief wait for async completion (max ~100ms to avoid blocking game thread too long)
		constexpr int32 MaxWaitIterations = 100;
		int32 WaitCount = 0;
		while (!Sensor->IsSensorTaskWorkDone() && WaitCount < MaxWaitIterations)
		{
			FPlatformProcess::Sleep(0.001f);
			++WaitCount;
		}
		
		if (WaitCount >= MaxWaitIterations)
		{
			UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SENSOR] RemoveSensor: Timeout waiting for async task on '%s'"), *SensorTag.ToString());
		}
	}
	
	ActiveSensors.Remove(SensorTag);

	// Determine sensor type for DestroySensor call
	if (OwnerInteraction && Sensor)
	{
		ESensorType SensorType = ESensorType::Active;
		if (Cast<UPassiveSensor>(Sensor))
		{
			SensorType = ESensorType::Passive;
		}

		OwnerInteraction->DestroySensor(SensorType, SensorTag);
	}

	return true;
}

void UHarmoniaSenseComponent::RemoveAllSensors()
{
	TArray<FName> SensorTags;
	ActiveSensors.GetKeys(SensorTags);

	for (const FName& Tag : SensorTags)
	{
		RemoveSensor(Tag);
	}
}

void UHarmoniaSenseComponent::OnEquipmentChanged(const TArray<FName>& NewSensorConfigs)
{
	// Remove all current sensors
	RemoveAllSensors();

	// Add new sensors from config list
	for (const FName& ConfigRowName : NewSensorConfigs)
	{
		AddSensor(ConfigRowName);
	}
}

USensorBase* UHarmoniaSenseComponent::GetSensor(FName SensorTag) const
{
	const TObjectPtr<USensorBase>* Found = ActiveSensors.Find(SensorTag);
	return Found ? Found->Get() : nullptr;
}

TArray<USensorBase*> UHarmoniaSenseComponent::GetAllSensors() const
{
	TArray<USensorBase*> Result;
	for (const auto& Pair : ActiveSensors)
	{
		if (Pair.Value)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FHarmoniaSenseConfigData UHarmoniaSenseComponent::GetSenseConfigByRowName(FName RowName) const
{
	UDataTable* SenseConfigDT = UHarmoniaDataTableBFL::GetSenseConfigDataTable();
	if (!SenseConfigDT)
	{
		return FHarmoniaSenseConfigData();
	}

	FHarmoniaSenseConfigData* ConfigPtr = SenseConfigDT->FindRow<FHarmoniaSenseConfigData>(RowName, TEXT("GetSenseConfigByRowName"));
	return ConfigPtr ? *ConfigPtr : FHarmoniaSenseConfigData();
}

void UHarmoniaSenseComponent::TriggerManualSensors()
{
	for (const auto& Pair : ActiveSensors)
	{
		if (USensorBase* Sensor = Pair.Value)
		{
			// Only trigger Manual sensors - Active sensors auto-update via timer
			if (Sensor->SensorType == ESensorType::Manual && Sensor->bEnable)
			{
				Sensor->UpdateSensor();
			}
		}
	}
}

// ============================================================================
// Initialization
// ============================================================================
void UHarmoniaSenseComponent::InitializeOwnerSenseComponents()
{
	EnsureOwnerInteractable();
	EnsureOwnerInteraction();
}

void UHarmoniaSenseComponent::EnsureOwnerInteractable()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Already have a valid reference?
	if (OwnerInteractable && IsValid(OwnerInteractable))
	{
		RegisterStimulusResponsesFromDataTable();
		return;
	}

	// Try to find existing component
	OwnerInteractable = Owner->FindComponentByClass<UHarmoniaSenseInteractableComponent>();

	if (!OwnerInteractable)
	{
		OwnerInteractable = NewObject<UHarmoniaSenseInteractableComponent>(
			Owner,
			UHarmoniaSenseInteractableComponent::StaticClass(),
			FName("SenseInteractable"));

		if (OwnerInteractable)
		{
			OwnerInteractable->SetEnableSenseStimulus(false);
			Owner->AddInstanceComponent(OwnerInteractable);  // Add to Actor's component list
			OwnerInteractable->RegisterComponent();
			OwnerInteractable->Activate(true);  // Explicitly activate

			// Stimulus doesn't need attachment - it follows owner via StimulusMobility setting
			// Get StimulusMobility from DataTable (use first row as default)
			EStimulusMobility StimulusMobility = EStimulusMobility::MovableOwner;
			UDataTable* SenseConfigDT = UHarmoniaDataTableBFL::GetSenseConfigDataTable();
			if (SenseConfigDT)
			{
				TArray<FName> RowNames = SenseConfigDT->GetRowNames();
				if (RowNames.Num() > 0)
				{
					FHarmoniaSenseConfigData* ConfigPtr = SenseConfigDT->FindRow<FHarmoniaSenseConfigData>(RowNames[0], TEXT("EnsureOwnerInteractable"));
					if (ConfigPtr)
					{
						StimulusMobility = ConfigPtr->StimulusMobility;
					}
				}
			}
			OwnerInteractable->SetStimulusMobility(StimulusMobility);
			RegisterStimulusResponsesFromDataTable();
			OwnerInteractable->SetEnableSenseStimulus(true);
		}
	}
	else
	{
		RegisterStimulusResponsesFromDataTable();
	}
}

void UHarmoniaSenseComponent::EnsureOwnerInteraction()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Already have a valid reference? Just bind callbacks
	if (OwnerInteraction && IsValid(OwnerInteraction))
	{
		// Ensure callbacks are bound (might not be if called again)
		if (!OwnerInteraction->OnNewSense.IsAlreadyBound(this, &UHarmoniaSenseComponent::OnSenseDetected))
		{
			OwnerInteraction->OnNewSense.AddDynamic(this, &UHarmoniaSenseComponent::OnSenseDetected);
			OwnerInteraction->OnCurrentSense.AddDynamic(this, &UHarmoniaSenseComponent::OnSenseDetected);
		}
		return;
	}

	// Try to find existing component
	OwnerInteraction = Owner->FindComponentByClass<UHarmoniaSenseInteractionComponent>();

	if (!OwnerInteraction)
	{
		OwnerInteraction = NewObject<UHarmoniaSenseInteractionComponent>(
			Owner,
			UHarmoniaSenseInteractionComponent::StaticClass(),
			FName("SenseInteraction"));

		if (OwnerInteraction)
		{
			Owner->AddInstanceComponent(OwnerInteraction);  // Add to Actor's component list
			OwnerInteraction->RegisterComponent();
			OwnerInteraction->Activate(true);  // Explicitly activate
			
			if (Owner->GetRootComponent())
			{
				OwnerInteraction->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}
	}

	// Bind to sense detection callbacks
	if (OwnerInteraction)
	{
		OwnerInteraction->OnNewSense.AddDynamic(this, &UHarmoniaSenseComponent::OnSenseDetected);
		OwnerInteraction->OnCurrentSense.AddDynamic(this, &UHarmoniaSenseComponent::OnSenseDetected);
	}
}

void UHarmoniaSenseComponent::RegisterStimulusResponsesFromDataTable()
{
	if (!OwnerInteractable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SENSOR] RegisterStimulusResponsesFromDataTable: OwnerInteractable is null"));
		return;
	}

	UDataTable* SenseConfigDT = UHarmoniaDataTableBFL::GetSenseConfigDataTable();
	if (!SenseConfigDT)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SENSOR] RegisterStimulusResponsesFromDataTable: SenseConfigDataTable not found"));
		return;
	}

	TArray<FName> RowNames = SenseConfigDT->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FHarmoniaSenseConfigData* ConfigPtr = SenseConfigDT->FindRow<FHarmoniaSenseConfigData>(RowName, TEXT("RegisterStimulusResponses"));
		if (ConfigPtr && ConfigPtr->SensorClass)
		{
			// Get SensorTag and Channel from Sensor BP CDO
			USensorBase* SensorCDO = ConfigPtr->SensorClass->GetDefaultObject<USensorBase>();
			if (SensorCDO)
			{
				FName SensorTag = SensorCDO->SensorTag;
				
				// Skip if SensorTag is not set
				if (SensorTag.IsNone())
				{
					UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SENSOR] Skipping stimulus registration: SensorTag is None for SensorClass '%s'"),
						*ConfigPtr->SensorClass->GetName());
					continue;
				}
				
				// Get channels from sensor BP's ChannelSetup
				const TArray<FChannelSetup>& Channels = SensorCDO->ChannelSetup;
				for (const FChannelSetup& Ch : Channels)
				{
					OwnerInteractable->SetResponseChannel(SensorTag, Ch.Channel, true);
				}

				if (Channels.Num() == 0)
				{
					OwnerInteractable->SetResponseChannel(SensorTag, 1, true);
				}
				
				OwnerInteractable->SetScore(SensorTag, ConfigPtr->BaseScore);
				
				// Also register additional ResponseTags from DataTable (same channels)
				for (const FName& ResponseTag : ConfigPtr->ResponseTags)
				{
					for (const FChannelSetup& Ch : Channels)
					{
						OwnerInteractable->SetResponseChannel(ResponseTag, Ch.Channel, true);
					}
					if (Channels.Num() == 0)
					{
						OwnerInteractable->SetResponseChannel(ResponseTag, 1, true);
					}
					OwnerInteractable->SetScore(ResponseTag, ConfigPtr->BaseScore);
				}
			}
		}
	}
}

void UHarmoniaSenseComponent::InitializeSensorsFromDataTable()
{
	UDataTable* SenseConfigDT = UHarmoniaDataTableBFL::GetSenseConfigDataTable();
	if (!SenseConfigDT)
	{
		return;
	}

	TArray<FName> RowNames = SenseConfigDT->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		AddSensor(RowName);
	}
}

// ============================================================================
// SenseSystem Callbacks
// ============================================================================

void UHarmoniaSenseComponent::OnSenseDetected(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli)
{
	if (!SensorPtr || SensedStimuli.Num() == 0)
	{
		return;
	}

	// Only process hits during attack window (managed by MeleeCombatComponent)
	AActor* Owner = GetOwner();
	if (Owner)
	{
		if (UHarmoniaMeleeCombatComponent* CombatComp = Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>())
		{
			if (!CombatComp->IsInAttackWindow())
			{
				return;
			}
		}
	}

	// Forward to delegate for combat components to process
	OnSenseHit.Broadcast(SensorPtr, Channel, SensedStimuli);
}
