#pragma once
#include <vector>

class InteractiveShape;
class RenderShape;

enum Axis
{
	X_Axis,
	Y_Axis
};

enum Child
{
	Left,
	Right,
	Root
};

struct KDTreeNode
{
	// Render shape object for displaying this node
	RenderShape* divider;
	// The axis along which this node makes its division
	Axis axis;
	// The location on the axis at which the division is made
	float axisValue;
	// Indicies for left and right children of this node
	int left;
	int right;
	// Index for this node's parent
	int parent;
	// Which child (Left or right) this node is to its parent
	Child child;
	// Whether or not this node should be displayed to the screen
	bool active;
	// What level of subdivision this node exists at
	int depth;
	// The child number of this node in this node's subdivision
	int branchMod;
	// This node's location in the entire node array
	int index;
	// The range of objects that this node has within its division
	int start;
	int end;
	// The beginning and ending locations of the visual line showing this node's division
	float lineStart;
	float lineEnd;
};

class KDTreeManager
{
public:

	static void InitKDTree(int maxDepth, RenderShape &lineTemplate);

	static void UpdateKDtree();

	static void AddShape(InteractiveShape* shape);

	static void DumpData();

	static void GetNearbyShapes(InteractiveShape* shape, std::vector<InteractiveShape*>& shapeVec);

	static void SetMaxDepth(int newMaxDepth);

	static int maxDepth();

private:

	static KDTreeNode* InitNode(int depth, int parentIndex, int branchMod, int index, Child child, Axis axis);

	static void DeactivateNode(KDTreeNode* node);

	static void ActivateNode(KDTreeNode* node, float axisValue);

	static int GetDepthIndex(int depth);

	static std::vector<KDTreeNode*> _kdTree;
	static std::vector<InteractiveShape*> _shapes;
	static int _maxDepth;
	static int _maxMaxDepth;
	static RenderShape _lineTemplate;
};
