// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
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

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

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
	
	UPROPERTY()
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY()
	class UPlayerAttackComponent* PlayerAttackComponent;
	
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
	int32 JumpCount = 0; // 점프 횟수
	UPROPERTY(EditAnywhere, Category = "Jump")
	int32 MaxJumpCount = 2; // 최대 점프 횟수 제한
	
	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MoveSpeed = 400.0f;

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

	UFUNCTION(Category="Movement")
	void ResetMovementParams() const; // 마찰력 복원 함수
	FVector2D LastMovementInput; // 마지막 이동 입력 저장

	UFUNCTION()
	void RechargeDash(); // 대쉬 충전 함수 선언
	
	// 대시 시스템 변수
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	int32 MaxDashCount = 3;
	UPROPERTY(VisibleAnywhere, Category = "Movement", Replicated)
	int32 CurrentDashCount = 3;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DashCooldown = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DashRechargeTime = 3.0f;

	FTimerHandle DashPhysicsTimerHandle; // 물리 설정 복원 전용
	FTimerHandle DashCooldownTimerHandle; // 대쉬 쿨타임
	FTimerHandle DashRechargeTimerHandle; // 대쉬 차지

	UPROPERTY(EditDefaultsOnly, Category = "Movement", Replicated)
	bool bCanDash = true;
	
	// 근접 공격 함수 선언
	void MeleeAttack(const FInputActionValue& Value);
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};

