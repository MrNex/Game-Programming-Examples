#include "KDTreeManager.h"
#include "InteractiveShape.h"
#include "RenderShape.h"
#include "RenderManager.h"

#include <stack>

std::vector<KDTreeNode*> KDTreeManager::_kdTree;
std::vector<InteractiveShape*> KDTreeManager::_shapes;
int KDTreeManager::_maxDepth;
int KDTreeManager::_maxMaxDepth;
RenderShape KDTreeManager::_lineTemplate;

// As with the octtreen and quadtree, the entire tree is instantiated when init is called. Unlike the previous trees, the 
// entire tree will be used to sort the array of shapes. In the case of this particular demo however, the max depth of the
// tree can be changed, so some of the nodes will be inactive if they are beyond the current max depth.
void KDTreeManager::InitKDTree(int maxDepth, RenderShape &lineTemplate)
{

	_maxDepth = maxDepth;
	_maxMaxDepth = maxDepth;
	_lineTemplate = lineTemplate;
	_kdTree.resize(GetDepthIndex(_maxDepth));

	std::stack<KDTreeNode*> stack = std::stack<KDTreeNode*>();
	stack.push(InitNode(0, -1, 0, 0, Root, X_Axis));

	while (!stack.empty())
	{
		KDTreeNode* node = stack.top();
		stack.pop();

		_kdTree[node->index] = node;

		if (node->parent >= 0)
		{
			if (node->child == Left)
			{
				_kdTree[node->parent]->left = node->index;
			}
			else
			{
				_kdTree[node->parent]->right = node->index;
			}
		}
		if (node->depth != _maxDepth)
		{
			int mod = node->parent != -1 ? node->branchMod * 2 : 0;
			int depthIndex = GetDepthIndex(node->depth);
			int index = depthIndex + mod;
			Axis axis = node->axis == X_Axis ? Y_Axis : X_Axis;
			stack.push(InitNode(node->depth + 1, node->index, index - depthIndex, index, Left, axis));
			stack.push(InitNode(node->depth + 1, node->index, index + 1 - depthIndex, index + 1, Right, axis));
		}
	}
}

void BubbleSort(std::vector<InteractiveShape*>& vector, int start, int end, Axis axis)
{
	bool sorted = false;
	while (!sorted)
	{
		sorted = true;
		int size = (int)vector.size();
		for (int i = start; i < end && i < size - 1; ++i)
		{
			// Check whether the two current values are in lesser to greater order
			if (axis == X_Axis && vector[i]->transform().position.x > vector[i + 1]->transform().position.x)
			{
				InteractiveShape* temp = vector[i + 1];
				vector[i + 1] = vector[i];
				vector[i] = temp;
				sorted = false;
			}
			else if (axis == Y_Axis && vector[i]->transform().position.y > vector[i + 1]->transform().position.y)
			{
				InteractiveShape* temp = vector[i + 1];
				vector[i + 1] = vector[i];
				vector[i] = temp;
				sorted = false;
			}
		}
	}
}

// For each node of the K-D tree, each node is deactivated and the shapes are sorted back into the tree.
// Each node possesses a beginning and an ending index. The shapes in the array between these values are 
// sorted based on the sorting axis of the current node of the tree. 
void KDTreeManager::UpdateKDtree()
{
	unsigned int size = _kdTree.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		DeactivateNode(_kdTree[i]);
	}

	std::stack<int> startStack = std::stack<int>();
	startStack.push(0);
	std::stack<int> endStack = std::stack<int>();
	endStack.push(_shapes.size() - 1);
	std::stack<int> nodeStack = std::stack<int>();
	nodeStack.push(0);

	// To avoid messy recursion, node starting and ending values are stored in stacks.
	// One level of the stack represents data for a single node. Since this system uses
	// a stack and not a queue, the tree is build depth-first. 
	while (!startStack.empty())
	{
		int start = startStack.top();
		startStack.pop();
		int end = endStack.top();
		endStack.pop();
		int node = nodeStack.top();
		nodeStack.pop();

		_kdTree[node]->start = start;
		_kdTree[node]->end = end;

		BubbleSort(_shapes, start, end, _kdTree[node]->axis);

		// Now that the vector is sorted, find the median
		int medianIndex = start + (end - start) / 2;
		if (_kdTree[node]->axis == X_Axis) ActivateNode(_kdTree[node], _shapes[medianIndex]->transform().position.x);
		if (_kdTree[node]->axis == Y_Axis) ActivateNode(_kdTree[node], _shapes[medianIndex]->transform().position.y);

		if (_kdTree[node]->depth < _maxDepth && medianIndex != start && medianIndex != end)
		{
			startStack.push(start);
			endStack.push(medianIndex - 1);
			nodeStack.push(_kdTree[node]->left);

			startStack.push(medianIndex + 1);
			endStack.push(end);
			nodeStack.push(_kdTree[node]->right);
		}
	}

}

void KDTreeManager::AddShape(InteractiveShape* shape)
{
	_shapes.push_back(shape);
}

void KDTreeManager::DumpData()
{
	int i;
	while ((i = _kdTree.size()) > 0)
	{
		delete _kdTree[i - 1];
		_kdTree.pop_back();
	}
}

// This function represents the main advantage of using a K-D tree, and that is searching. A K-D tree allows for binary
// searching when dealing with multiple dividng variables. 
void KDTreeManager::GetNearbyShapes(InteractiveShape* shape, std::vector<InteractiveShape*>& shapeVec)
{
	unsigned int size = _kdTree.size();
	float pos;
	KDTreeNode* node;
	int numShapes, start, end, medianIndex;
	bool xAxis;
	for (unsigned int i = 0; i < size;)
	{
		node = _kdTree[i];
		xAxis = node->axis == X_Axis;
		pos = xAxis ? shape->transform().position.x : shape->transform().position.y;
		// Go down the tree until we have a hit unless we hit a dividing shape. 
		if (pos != node->axisValue && node->depth < _maxDepth)
			i = pos < node->axisValue ? node->left : node->right;
		else
		{
			medianIndex = node->start + (node->end - node->start) / 2;
			// Decide which side of the median we're on and return all of the shapes on the side we're on.
			// But if we're actually on the median, then return both sides.
			if (pos <= node->axisValue)
			{
				start = node->start;
				end = medianIndex - 1;
				if (pos == node->axisValue)
					end = node->end;
			}
			else
			{
				start = medianIndex + 1;
				end = node->end;
			}

			numShapes = end - start + 1;
			shapeVec.resize(numShapes);
			for (int j = 0; j < numShapes; ++j)
			{
				shapeVec[j] = _shapes[start + j];
			}
			break;
		}
	}
}

KDTreeNode* KDTreeManager::InitNode(int depth, int parentIndex, int branchMod, int index, Child child, Axis axis)
{
	KDTreeNode* node = new KDTreeNode();
	node->axis = axis;
	node->axisValue = 0.0f;
	node->left = -1;
	node->right = -1;
	node->parent = parentIndex;
	node->child = child;
	node->active = false;
	node->depth = depth;
	node->branchMod = branchMod;
	node->index = index;

	RenderShape* line = new RenderShape(_lineTemplate.vao(), _lineTemplate.count(), _lineTemplate.mode(), _lineTemplate.shader(), _lineTemplate.color());
	RenderManager::AddShape(line);
	node->divider = line;
	node->start = 0;
	node->end = 0;

	return node;
}

void KDTreeManager::DeactivateNode(KDTreeNode* node)
{
	node->active = false;
	node->divider->active() = false;
	node->axisValue = 0.0f;
}

// Most of this code is here for defining the transforms of the dividing lines, entirely aesthetic
void KDTreeManager::ActivateNode(KDTreeNode* node, float axisValue)
{
	node->active = true;
	node->divider->active() = true;
	if (node->axis == X_Axis)
	{
		float top;
		float bottom;

		switch (node->child)
		{
		case Left:
			top = _kdTree[node->parent]->axisValue;
			if (node->depth > 2)
			{
				bottom = _kdTree[_kdTree[node->parent]->parent]->lineStart;
			}
			else
				bottom = -1.0f;
			break;
		case Right:
			bottom = _kdTree[node->parent]->axisValue;
			if (node->depth > 2)
			{
				top = _kdTree[_kdTree[node->parent]->parent]->lineEnd;
			}
			else
				top = 1.0f;
			break;
		case Root:
			top = 1.0f;
			bottom = -1.0f;
			break;
		}

		node->lineStart = bottom;
		node->lineEnd = top;

		node->divider->transform().rotation = glm::angleAxis(45.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		node->divider->transform().position = glm::vec3(axisValue, (top + bottom) / 2.0f, 0.0f);
		node->divider->transform().scale = glm::vec3(1.0f, (top - bottom) / 1.4142136f / 2.0f, 1.0f);
	}
	else
	{
		float right;
		float left;

		switch (node->child)
		{
		case Left:
			right = _kdTree[node->parent]->axisValue;
			if (node->depth > 2)
			{
				left = _kdTree[_kdTree[node->parent]->parent]->lineStart;
			}
			else
				left = -1.337f;
			break;
		case Right:
			left = _kdTree[node->parent]->axisValue;
			if (node->depth > 2)
			{
				right = _kdTree[_kdTree[node->parent]->parent]->lineEnd;
			}
			else
				right = 1.337f;
			break;
		case Root:
			left = -1.337f;
			right = 1.337f;
			break;
		}

		node->lineStart = left;
		node->lineEnd = right;

		node->divider->transform().rotation = glm::angleAxis(-45.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		node->divider->transform().position = glm::vec3((left + right) / 2.0f, axisValue, 0.0f);
		node->divider->transform().scale = glm::vec3((right - left) / 1.4142136f / 2.0f, 1.0f, 1.0f);
	}
	node->axisValue = axisValue;
}

int KDTreeManager::GetDepthIndex(int depth)
{
	float depthIndex = 1.0f;
	for (int i = 1; i <= depth; ++i)
	{
		depthIndex += powf(2.0f, i);
	}

	return (int)depthIndex;
}

void KDTreeManager::SetMaxDepth(int newMaxDepth)
{
	if (newMaxDepth >= 0 && newMaxDepth != _maxDepth && newMaxDepth <= _maxMaxDepth)
	{
		_maxDepth = newMaxDepth;
		UpdateKDtree();
	}
}

int KDTreeManager::maxDepth()
{
	return _maxDepth;
}


