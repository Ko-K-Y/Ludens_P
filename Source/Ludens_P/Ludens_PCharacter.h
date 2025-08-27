// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Ludens_PCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ALudens_PCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;
public:
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
private:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// 대쉬 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DashAction;

	// 근접 공격 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MeleeAttackAction;

	// 테스트 공격 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* TestAttackAction;

	// 재장전 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;

	// 무기 공격 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	// 팀원 소생 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReviveAction;

	// 젤루 흡수 Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AbsorbAction;
	
	UPROPERTY()
	class UInputMappingContext* DefaultMappingContext;
	UPROPERTY()
	class UPlayerAttackComponent* PlayerAttackComponent;
	UPROPERTY()
	class UPlayerStateComponent* PlayerStateComponent;
	UPROPERTY()
	class UTP_WeaponComponent* WeaponComponent;
	UPROPERTY()
	class UReviveComponent* ReviveComponent;
	
public:
	ALudens_PCharacter();

protected:
	virtual void BeginPlay();

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

protected:
	void TestAttack(const FInputActionValue& Value);
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

private:
	UPROPERTY(Replicated)
	int8 JumpCount = 0; // 점프 횟수
	UPROPERTY(EditAnywhere, Category = "Jump")
	int8 MaxJumpCount = 2; // 최대 점프 횟수 제한

	// 대쉬 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float DashSpeed = 1500.0f;
	
public:
	void Landed(const FHitResult& Hit) override; // 땅에 착지 했는지 안 했는지 판단

protected:
	UFUNCTION(Server, Reliable)
	void Server_Jump();
	// Jump 함수 선언
	virtual void Jump() override;
	
	UFUNCTION(Server, Reliable)
	void Server_Dash();
	// Dash 함수 선언
	void Dash(const FInputActionValue& Value);
	FTimerHandle DashTimerHandle; // 대시 타이머 핸들
    
	// 마찰력 원본 값 저장용 변수
	float OriginalGroundFriction = 8.0f;
	float OriginalBrakingDeceleration = 2048.0f;

	UFUNCTION(Category="Dash")
	void ResetMovementParams() const; // 마찰력 복원 함수
	FVector2D LastMovementInput; // 마지막 이동 입력 저장

	UFUNCTION()
	void RechargeDash(); // 대쉬 충전 함수 선언
	
	// 대시 시스템 변수
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	int8 MaxDashCount = 3;
	UPROPERTY(VisibleAnywhere, Category = "Dash", Replicated)
	int8 CurrentDashCount = 3;
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashCooldown = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashRechargeTime = 3.0f;

	FTimerHandle DashPhysicsTimerHandle; // 물리 설정 복원 전용
	FTimerHandle DashCooldownTimerHandle; // 대쉬 쿨타임
	FTimerHandle DashRechargeTimerHandle; // 대쉬 차지

	UPROPERTY(EditDefaultsOnly, Category = "Dash", Replicated)
	bool bCanDash = true;

	// 근접 공격인지 부활인지 판단하는 함수 선언
	void Interact(const FInputActionValue& Value);
	
	// 근접 공격 함수 선언
	void MeleeAttack(const FInputActionValue& Value);
	
	// 무기 공격 함수 선언
	UFUNCTION(Server, Reliable)
	void Server_Fire(const FInputActionValue& Value);
	void Fire(const FInputActionValue& Value);
	
	// 재장전 함수 선언
	UFUNCTION(Server, Reliable)
	void Server_Reload();
	void Reload(const FInputActionValue& Value);
	void HandleReload();
	// 재장전 시스템 변수
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	int16 MaxSavedAmmo = 500;
public:
	UPROPERTY(EditDefaultsOnly, Category = "Reload", ReplicatedUsing = OnRep_SavedAmmo)
	int16 SavedAmmo = 100;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	int16 MaxAmmo = 10;
	UPROPERTY(EditDefaultsOnly, Category = "Reload", ReplicatedUsing = OnRep_CurrentAmmo)
	int16 CurrentAmmo = 10;
	
	UFUNCTION()
	void OnRep_SavedAmmo();
	UFUNCTION()
	void OnRep_CurrentAmmo();
public:
	int16 GetCurrentAmmo() const;

protected:
	UFUNCTION(Server, Reliable)
	void Server_Revive();
	void Revive(const FInputActionValue& Value);

	void Absorb(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void Server_Absorb(const FInputActionValue& Value);
	void AbsorbComplete(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void Server_AbsorbComplete(const FInputActionValue& Value);
	FTimerHandle AbsorbCompleteTimerHandle;
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};

