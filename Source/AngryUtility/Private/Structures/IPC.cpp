// Maintained by AngryLizard, netliz.net

#include "Structures/IPC.h"

FVector4 FIPCPendulumProperties::PositionToPendulum(const FIPCPosition& Position, const FVector& UpDirection, const FVector& ForwardDirection) const
{
	FVector4 State;

	const FVector Axis = Position.Rotation.GetAxisZ();
	const float Dot = ForwardDirection | Axis;
	State[2] = FMath::Acos(Dot);
	State[0] = (ForwardDirection | Position.Position) - Dot * Length;

	return State;
}

FVector4 FIPCPendulumProperties::DynamicToPendulum(const FIPCDynamic& Dynamic, const FVector& UpDirection, const FVector& ForwardDirection) const
{
	FVector4 State = PositionToPendulum(Dynamic, UpDirection, ForwardDirection);

	const FVector Axis = Dynamic.Rotation.GetAxisZ();
	const FVector Right = ForwardDirection ^ UpDirection;
	State[3] = Right | Dynamic.Angular;
	State[1] = (ForwardDirection | Dynamic.Velocity) - State[3] * Length;

	return State;
}

FIPCPendulumState FIPCPendulumProperties::PositionToPendulum(const FIPCPosition& Position) const
{
	FIPCPendulumState State;
	const FVector Axis = Position.Rotation.GetAxisZ();

	State.X[2] = FMath::Acos(Axis | FVector::RightVector);
	State.Y[2] = FMath::Acos(Axis | FVector::ForwardVector);
	State.R[0] = Position.Rotation.GetTwistAngle(Axis);

	State.X[0] = Position.Position.X - Axis.X * Length;
	State.Y[0] = Position.Position.Y - Axis.Y * Length;
	State.Z[0] = Position.Position.Z - Axis.Z * Length;
	return State;
}

FIPCPendulumState FIPCPendulumProperties::DynamicToPendulum(const FIPCDynamic& Dynamic) const
{
	FIPCPendulumState State = PositionToPendulum(Dynamic);

	State.X[3] = Dynamic.Angular | FVector::RightVector;
	State.Y[3] = Dynamic.Angular | FVector::ForwardVector;
	State.R[1] = Dynamic.Angular | FVector::UpVector;

	const FVector Axis = Dynamic.Rotation.GetAxisZ();
	const FVector Radial = (Axis * Length) ^ Dynamic.Angular;
	State.X[1] = Dynamic.Velocity.X - Radial.X;
	State.Y[1] = Dynamic.Velocity.Y - Radial.Y;
	State.Z[1] = Dynamic.Velocity.Z - Radial.Z;
	return State;
}

FIPCPosition FIPCPendulumProperties::PendulumToPosition(const FIPCPendulumState& State) const
{
	FIPCPosition Position;
	Position.Rotation = FQuat(FVector::RightVector, State.X[2]) * FQuat(FVector::ForwardVector, State.Y[2]) * FQuat(FVector::UpVector, State.R[0]);

	const FVector Axis = Position.Rotation.GetAxisZ();
	Position.Position = FVector(State.X[0], State.Y[0], State.Z[0]) + Axis * Length;
	return Position;
}

FIPCDynamic FIPCPendulumProperties::PendulumToDynamic(const FIPCPendulumState& State) const
{
	FIPCPosition Position = PendulumToPosition(State);

	FIPCDynamic Dynamic;
	Dynamic.Position = Position.Position;
	Dynamic.Rotation = Position.Rotation;
	Dynamic.Velocity = FVector::RightVector * State.X[3] + FVector::ForwardVector * State.Y[3] + FVector::UpVector * State.R[1];

	const FVector Axis = Dynamic.Rotation.GetAxisZ();
	const FVector Radial = (Axis * Length) ^ Dynamic.Angular;
	Dynamic.Angular = FVector(State.X[1], State.Y[1], State.Z[1]) + Radial;
	return Dynamic;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FIPCProperties::Generate(const FIPCPendulumProperties& PendulumProperties, const FIPCRiccatiProperties& RiccatiProperties)
{
	const float L = PendulumProperties.Length;
	const float M = PendulumProperties.Mass;
	const float I = PendulumProperties.Inertia;
	const float G = PendulumProperties.Gravity;

	const float LinvI = L / I;
	const float invM = 1.0f / M;
	const float mgL = M * G * L * LinvI;

	StateCoef = FMatrix();
	StateCoef.M[0][1] = 1.0f;
	StateCoef.M[1][2] = mgL * L;
	StateCoef.M[2][3] = 1.0f;
	StateCoef.M[3][2] = mgL;

	ForceCoef = FVector4();
	ForceCoef[1] = invM + LinvI * L;
	ForceCoef[3] = LinvI;

	const FMatrix A = StateCoef;
	const FMatrix AT = StateCoef.GetTransposed();

	const FVector4 B = ForceCoef;

	FMatrix Q;
	Q.M[0][0] = RiccatiProperties.Q;
	Q.M[1][1] = RiccatiProperties.Q;
	Q.M[2][2] = RiccatiProperties.Q;
	Q.M[3][3] = RiccatiProperties.Q;

	const float Dt = RiccatiProperties.Dt;
	const float Rinv = 1.0f / RiccatiProperties.R;

	FMatrix P = Q;

	// Source from https://github.com/TakaHoribe/Riccati_Solver/blob/master/riccati_solver.cpp
	for (int32 Iteration = 0; Iteration < RiccatiProperties.Iterations; Iteration++)
	{
		P = P + (P * A + AT * P + P * -Dot4(B * Rinv, B) * P + Q) * Dt;
	}

	ForceResponse = P.GetTransposed().TransformFVector4(B) * Rinv;
}

void FIPCProperties::SimulateForPosition(FVector4& State, float Position, float DeltaTime) const
{
	const float Force = Dot4(ForceResponse, State - FVector4(Position, State.Y, State.Z, State.W));
	const FVector4 DState = StateCoef.TransformFVector4(State) + ForceCoef * Force;
	State += DState * DeltaTime;
}

void FIPCProperties::SimulateForVelocity(FVector4& State, float Velocity, float DeltaTime) const
{
	const float Force = Dot4(ForceResponse, State - FVector4(State.X, Velocity, State.Z, State.W));
	const FVector4 DState = StateCoef.TransformFVector4(State) + ForceCoef * Force;
	State += DState * DeltaTime;
}
