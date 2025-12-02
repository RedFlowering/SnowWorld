// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaRecoveryItemDefinitions.h"
#include "HarmoniaRechargeableItemComponent.generated.h"

class APlayerController;
class UHarmoniaCheckpointSubsystem;

/**
 * Rechargeable Item Events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemUsed, EHarmoniaRecoveryItemType, ItemType, int32, RemainingCharges, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemRecharged, EHarmoniaRecoveryItemType, ItemType, int32, CurrentCharges, int32, MaxCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChargesChanged, EHarmoniaRecoveryItemType, ItemType, int32, NewCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpgraded, EHarmoniaRecoveryItemType, ItemType, int32, NewMaxCharges);

/**
 * UHarmoniaRechargeableItemComponent
 *
 * ì¶©ì „ ê°€?¥í•œ ?Œë³µ ?„ì´?œì„ ê´€ë¦¬í•˜??ì»´í¬?ŒíŠ¸
 * ëª¨ë“  ?Œë³µ ?„ì´???€?…ì— ?¬ì‚¬??ê°€?¥í•œ ë²”ìš© ì»´í¬?ŒíŠ¸
 *
 * ì£¼ìš” ê¸°ëŠ¥:
 * - ?„ì´??ì¶©ì „ ?Ÿìˆ˜ ê´€ë¦?
 * - ì²´í¬?¬ì¸?¸ì—???ë™ ì¶©ì „
 * - ?„ì´???¬ìš© ê²€ì¦?
 * - ?„ì´???…ê·¸?ˆì´??(ìµœë? ì¶©ì „ ?Ÿìˆ˜ ì¦ê?)
 * - ?¤íŠ¸?Œí¬ ë¦¬í”Œë¦¬ì??´ì…˜
 * - ?€??ë¡œë“œ ?œìŠ¤???°ë™
 *
 * ?¬ì‚¬??ê°€?¥ì„±:
 * - ?Œë³µ ?„ì´???¸ì—??ì¶©ì „???„ì´?œì— ?¬ìš© ê°€??
 * - ?˜ì „ ?…ì¥ê¶? ë¶€??? í° ???•ì¥ ê°€??
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaRechargeableItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaRechargeableItemComponent();

	// UActorComponent interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End of UActorComponent interface

	// ============================================================================
	// Item Registration
	// ============================================================================

	/**
	 * ?Œë³µ ?„ì´???±ë¡ (ì´ˆê¸°??
	 * @param ItemType ?„ì´???€??
	 * @param Config ?„ì´???¤ì •
	 * @return ?±ë¡ ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool RegisterRecoveryItem(EHarmoniaRecoveryItemType ItemType, const FHarmoniaRecoveryItemConfig& Config);

	/**
	 * ê³µëª… ?Œí¸ ?±ë¡ (ì£¼íŒŒ?˜ë³„)
	 * @param Frequency ê³µëª… ì£¼íŒŒ??
	 * @param VariantConfig ?Œí¸ ?¤ì •
	 * @return ?±ë¡ ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool RegisterResonanceShard(EHarmoniaResonanceFrequency Frequency, const FHarmoniaResonanceShardVariant& VariantConfig);

	/**
	 * ?„ì´???±ë¡ ?´ì œ
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void UnregisterRecoveryItem(EHarmoniaRecoveryItemType ItemType);

	// ============================================================================
	// Item Usage
	// ============================================================================

	/**
	 * ?Œë³µ ?„ì´???¬ìš© (ì¶©ì „ ?Ÿìˆ˜ 1 ?Œëª¨)
	 * @param ItemType ?¬ìš©???„ì´???€??
	 * @return ?¬ìš© ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UseRecoveryItem(EHarmoniaRecoveryItemType ItemType);

	/**
	 * ê³µëª… ?Œí¸ ?¬ìš© (ì£¼íŒŒ?˜ë³„)
	 * @param Frequency ?¬ìš©???Œí¸??ê³µëª… ì£¼íŒŒ??
	 * @return ?¬ìš© ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UseResonanceShard(EHarmoniaResonanceFrequency Frequency);

	/**
	 * ?„ì´???¬ìš© ê°€???¬ë? ?•ì¸
	 * @param ItemType ?•ì¸???„ì´???€??
	 * @param OutReason ?¬ìš© ë¶ˆê??¥í•œ ?´ìœ  (?¤íŒ¨ ??
	 * @return ?¬ìš© ê°€???¬ë?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool CanUseRecoveryItem(EHarmoniaRecoveryItemType ItemType, FText& OutReason) const;

	/**
	 * ?¨ì? ì¶©ì „ ?Ÿìˆ˜ ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	int32 GetRemainingCharges(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * ìµœë? ì¶©ì „ ?Ÿìˆ˜ ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	int32 GetMaxCharges(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * ?„ì´???¤ì • ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool GetRecoveryItemConfig(EHarmoniaRecoveryItemType ItemType, FHarmoniaRecoveryItemConfig& OutConfig) const;

	/**
	 * ê³µëª… ?Œí¸ ?¤ì • ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool GetResonanceShardVariant(EHarmoniaResonanceFrequency Frequency, FHarmoniaResonanceShardVariant& OutVariant) const;

	// ============================================================================
	// Item Recharge
	// ============================================================================

	/**
	 * ?„ì´??ì¶©ì „
	 * @param ItemType ì¶©ì „???„ì´???€??
	 * @param Amount ì¶©ì „???Ÿìˆ˜ (-1?´ë©´ ìµœë?ì¹˜ê¹Œì§€ ì¶©ì „)
	 * @return ì¶©ì „???Ÿìˆ˜
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeItem(EHarmoniaRecoveryItemType ItemType, int32 Amount = -1);

	/**
	 * ëª¨ë“  ?„ì´??ì¶©ì „ (ì²´í¬?¬ì¸?¸ì—??ê³µëª… ??
	 * @return ì¶©ì „??ì´??Ÿìˆ˜
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeAllItems();

	/**
	 * ì¶©ì „ ê°€?¥í•œ ?„ì´?œë§Œ ì¶©ì „
	 * @return ì¶©ì „??ì´??Ÿìˆ˜
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeRechargeableItems();

	// ============================================================================
	// Item Upgrade
	// ============================================================================

	/**
	 * ?„ì´???…ê·¸?ˆì´??(ìµœë? ì¶©ì „ ?Ÿìˆ˜ ì¦ê?)
	 * @param ItemType ?…ê·¸?ˆì´?œí•  ?„ì´???€??
	 * @param AdditionalMaxCharges ì¦ê??œí‚¬ ìµœë? ì¶©ì „ ?Ÿìˆ˜
	 * @return ?…ê·¸?ˆì´???±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UpgradeItemMaxCharges(EHarmoniaRecoveryItemType ItemType, int32 AdditionalMaxCharges);

	// ============================================================================
	// Checkpoint Integration
	// ============================================================================

	/**
	 * ì²´í¬?¬ì¸??ê³µëª… ?´ë²¤???¸ë“¤??
	 */
	UFUNCTION()
	void OnCheckpointResonanceCompleted(FName CheckpointID, const FHarmoniaResonanceResult& Result);

	// ============================================================================
	// Data Persistence
	// ============================================================================

	/**
	 * ?„ì´???íƒœ ?€???°ì´??ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	TArray<FHarmoniaRecoveryItemState> GetItemStatesForSave() const;

	/**
	 * ?„ì´???íƒœ ë¡œë“œ
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void LoadItemStates(const TArray<FHarmoniaRecoveryItemState>& LoadedStates);

	// ============================================================================
	// Events
	// ============================================================================

	/** ?„ì´???¬ìš© ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemUsed OnItemUsed;

	/** ?„ì´??ì¶©ì „ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemRecharged OnItemRecharged;

	/** ì¶©ì „ ?Ÿìˆ˜ ë³€ê²??´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnChargesChanged OnChargesChanged;

	/** ?„ì´???…ê·¸?ˆì´???´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemUpgraded OnItemUpgraded;

protected:
	/**
	 * ?„ì´???±ë¡ ?¬ë? ?•ì¸
	 */
	bool IsItemRegistered(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * ì¶©ì „ ?Ÿìˆ˜ ë³€ê²?(?´ë??? ë¦¬í”Œë¦¬ì??´ì…˜ ?¬í•¨)
	 */
	void SetCharges(EHarmoniaRecoveryItemType ItemType, int32 NewCharges);

	/**
	 * ì²´í¬?¬ì¸???œë¸Œ?œìŠ¤??ì´ˆê¸°??
	 */
	void InitializeCheckpointIntegration();

private:
	/** ?±ë¡???Œë³µ ?„ì´???¤ì • */
	UPROPERTY(Transient)
	TMap<EHarmoniaRecoveryItemType, FHarmoniaRecoveryItemConfig> RegisteredItems;

	/** ?±ë¡??ê³µëª… ?Œí¸ ë³€??(ì£¼íŒŒ?˜ë³„) */
	UPROPERTY(Transient)
	TMap<EHarmoniaResonanceFrequency, FHarmoniaResonanceShardVariant> ResonanceShardVariants;

	/** ?„ì´?œë³„ ?„ì¬ ?íƒœ (?¤íŠ¸?Œí¬ ë¦¬í”Œë¦¬ì??´ì…˜) */
	UPROPERTY(ReplicatedUsing=OnRep_ItemStates)
	TArray<FHarmoniaRecoveryItemState> ItemStates;

	UFUNCTION()
	void OnRep_ItemStates();

	/** ì²´í¬?¬ì¸???œë¸Œ?œìŠ¤??ìºì‹œ */
	UPROPERTY(Transient)
	TObjectPtr<UHarmoniaCheckpointSubsystem> CheckpointSubsystem;
};
