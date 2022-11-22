// Maintained by AngryLizard, netliz.net

#include "Structures/SpatialTree.h"

FSpatialTree::FSpatialTree(const FVector& Location, const FVector& Size)
	: Location(Location), Size(Size), Space(FVector::ZeroVector)
{
}

FSpatialTree::~FSpatialTree()
{
}

bool FSpatialTree::IsInside(const FVector& Point) const
{
	return(Location.X <= Point.X && Point.X < Location.X + Size.X &&
		Location.Y <= Point.Y && Point.Y < Location.Y + Size.Y &&
		Location.Z <= Point.Z && Point.Z < Location.Z + Size.Z);
}

bool FSpatialTree::HasSpace(const FVector& Bounds) const
{
	return(Bounds.X < Space.X && Bounds.Y < Space.Y && Bounds.Z < Space.Z);
}

FVector FSpatialTree::GetMax(const FVector& Reference) const
{
	// If one component is bigger, all of them are
	return(Space.ComponentMax(Reference));
}


FSpatialLeaf::FSpatialLeaf(const FVector& Location, const FVector& Size)
	: FSpatialTree(Location, Size)
{
	// Occupied cells have no available space
	Space = FVector::ZeroVector;
}

FSpatialLeaf::~FSpatialLeaf()
{
}

bool FSpatialLeaf::Insert(const FVector& Bounds, FVector& Result)
{
	// Return min corner as cell location
	Result = Location + Size / 2;
	return(true);
}

bool FSpatialLeaf::Remove(const FVector& Point)
{
	return(true);
}

void FSpatialLeaf::ForEach(std::function<void(const FVector&, const FVector&, bool)> Func)
{
	const FVector Extend = Size / 2;
	const FVector Center = Location + Extend;
	Func(Center, Extend, true);
}


FSpatialBranch::FSpatialBranch(const FVector& Location, const FVector& Size, EAxis::Type Axis, int32 Slices)
	: FSpatialTree(Location, Size), Num(0), Slices(Slices), Axis(Axis)
{
	Children.SetNumZeroed(Slices);
	Space = SliceSize();
}

FSpatialBranch::~FSpatialBranch()
{
	// Free all children
	for (FSpatialTree*& Child : Children)
	{
		if (Child)
		{
			delete(Child);
			Child = nullptr;
		}
	}
}

EAxis::Type FSpatialBranch::GetNext() const
{
	switch (Axis)
	{
	case EAxis::Z: return(EAxis::Y);
	case EAxis::Y: return(EAxis::X);
	case EAxis::X: return(EAxis::Z);
	default: return(EAxis::None);
	}
}

FVector FSpatialBranch::SliceSize()
{
	// Slice length
	const float Length = Size.GetComponentForAxis(Axis);
	const float Section = Length / Slices;

	// Compute cell size
	FVector NewSize = Size;
	NewSize.SetComponentForAxis(Axis, Section);
	return(NewSize);
}

void FSpatialBranch::UpdateSpace()
{
	// Only update space if full
	if (Num == Slices)
	{
		// Reset space
		Space = FVector::ZeroVector;

		// Get max of all children
		for (FSpatialTree* Child : Children)
		{
			Space = Child->GetMax(Space);
		}
	}
	else
	{
		// Space is size of new slice
		Space = SliceSize();
	}
}

bool FSpatialBranch::Insert(FSpatialTree* Child, const FVector& Bounds, FVector& Result)
{
	bool Success = Child->Insert(Bounds, Result);
	UpdateSpace();
	return(Success);
}

bool FSpatialBranch::Insert(const FVector& Bounds, FVector& Result)
{
	// Allocate to already existing child
	for (FSpatialTree* Child : Children)
	{
		// Check if there is space and insert
		if (Child && Child->HasSpace(Bounds))
		{
			return(Insert(Child, Bounds, Result));
		}
	}

	// Compute next cell Dimensions
	const float Bound = Bounds.GetComponentForAxis(Axis);
	const float Length = Size.GetComponentForAxis(Axis);
	const float Section = Length / Slices;

	// Compute cell size
	FVector NewSize = Size;

	// Compute cell location
	FVector NewLocation = Location;
	const float Offset = Location.GetComponentForAxis(Axis);

	// Find empty slot
	for (int i = 0; i < Slices; i++)
	{
		if (!Children[i])
		{
			// Transform new dimensions
			NewSize.SetComponentForAxis(Axis, Section);
			NewLocation.SetComponentForAxis(Axis, Offset + Section * i);

			// Create branch
			EAxis::Type Next = GetNext();
			FSpatialTree* Child = new FSpatialBranch(NewLocation, NewSize, Next, Slices);

			// Determine whether cell can further be split
			if (!Child->HasSpace(Bounds))
			{
				// Create leaf instead
				delete(Child);
				Child = new FSpatialLeaf(NewLocation, NewSize);
			}

			Num++;
			Children[i] = Child;
			return(Insert(Child, Bounds, Result));
		}
	}

	return(false);
}

bool FSpatialBranch::Remove(const FVector& Point)
{
	// Allocate to already existing child
	for (FSpatialTree*& Child : Children)
	{
		// Check if point is inside child
		if (Child && Child->IsInside(Point) && Child->Remove(Point))
		{
			// Remove child
			delete(Child);
			Child = nullptr;
			Num--;
		}
	}

	// Delete if empty
	return(Num == 0);
}

void FSpatialBranch::ForEach(std::function<void(const FVector&, const FVector&, bool)> Func)
{
	// Call locally
	const FVector Extend = Size / 2;
	const FVector Center = Location + Extend;
	Func(Center, Extend, false);

	// Call for every child
	for (FSpatialTree* Child : Children)
	{
		if (Child)
		{
			Child->ForEach(Func);
		}
	}
}


FSpatialRoot::FSpatialRoot(const FVector& Location, const FVector& Size, int32 Slices)
	: FSpatialBranch(Location, Size, EAxis::Z, Slices)
{
}

FSpatialRoot::~FSpatialRoot()
{
}

bool FSpatialRoot::Insert(const FVector& Bounds, FVector& Result)
{
	if (HasSpace(Bounds))
	{
		return(FSpatialBranch::Insert(Bounds, Result));
	}
	return(false);
}