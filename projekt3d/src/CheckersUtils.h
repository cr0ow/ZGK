#ifndef CHECKERSUTILS_H
#define CHECKERSUTILS_H

#include <osg/Node>

using namespace osg;

void addHighlight(Node* node, const Vec4d &color);
void removeNodeHighlight(Node* node);
Vec3d getPawnPosition(const Node* pawn);
bool positionsEquals(const Vec3d& p1, const Vec3d& p2);

#endif //CHECKERSUTILS_H
