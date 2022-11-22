// Maintained by AngryLizard, netliz.net

#pragma once

#include "CoreMinimal.h"
#include <functional>

/**
 * 
 */
class ANGRYUTILITY_API FSpatialTree
{
protected:

	// Cell bounds
	FVector Location;
	FVector Size;

	// Biggest available space
	FVector Space;

public:
	FSpatialTree(const FVector& Location, const FVector& Size);
	virtual ~FSpatialTree();

	// Check whether bounds have room in this tree
	bool IsInside(const FVector& Point) const;

	// Check whether bounds have room in this tree
	bool HasSpace(const FVector& Bounds) const;

	// Get max bounds between Reference and Space
	FVector GetMax(const FVector& Reference) const;


	// Insert a box into this cell or one of its children
	virtual bool Insert(const FVector& Bounds, FVector& Result) = 0;

	// Remove a leaf at a location
	virtual bool Remove(const FVector& Point) = 0;

	// Calls for each child returning center, extend and whether it's a leaf
	virtual void ForEach(std::function<void(const FVector&, const FVector&, bool)> Func) = 0;
};

class ANGRYUTILITY_API FSpatialLeaf : public FSpatialTree
{
public:
	FSpatialLeaf(const FVector& Location, const FVector& Size);
	virtual ~FSpatialLeaf();
	
	virtual bool Insert(const FVector& Bounds, FVector& Result) override;
	virtual bool Remove(const FVector& Point) override;
	virtual void ForEach(std::function<void(const FVector&, const FVector&, bool)> Func) override;
};

class ANGRYUTILITY_API FSpatialBranch : public FSpatialTree
{
protected:

	// Children
	TArray<FSpatialTree*> Children;

	// Number of children
	int32 Num;

	// Max number of children
	int32 Slices;

	// Split axis
	EAxis::Type Axis;

public:
	FSpatialBranch(const FVector& Location, const FVector& Size, EAxis::Type Axis, int32 Slices);
	virtual ~FSpatialBranch();
	
	// Get next axis
	EAxis::Type GetNext() const;

	// Slice size
	FVector SliceSize();

	// Set max available space from children
	void UpdateSpace();

	// Inserts and updates 
	bool Insert(FSpatialTree* Child, const FVector& Bounds, FVector& Result);

	virtual bool Insert(const FVector& Bounds, FVector& Result) override;
	virtual bool Remove(const FVector& Point) override;
	virtual void ForEach(std::function<void(const FVector&, const FVector&, bool)> Func) override;
};

class ANGRYUTILITY_API FSpatialRoot : public FSpatialBranch
{
public:
	FSpatialRoot(const FVector& Location, const FVector& Size, int32 Slices);
	virtual ~FSpatialRoot();

	virtual bool Insert(const FVector& Bounds, FVector& Result) override;
};