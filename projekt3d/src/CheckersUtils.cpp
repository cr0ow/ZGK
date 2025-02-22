#include <osg/Node>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osg/Matrix>
#include "CheckersUtils.h"
#include "Commons.h"

using namespace std;
using namespace osg;

void addHighlight(Node* node, const Vec4d &color) {
    if (!node) return;
    const ref_ptr stateSet = node->getOrCreateStateSet();
    ref_ptr material = dynamic_cast<Material*>(stateSet->getAttribute(StateAttribute::MATERIAL));
    if (!material) {
        material = new Material();
        stateSet->setAttribute(material, StateAttribute::MATERIAL);
    }
    material->setDiffuse(Material::FRONT_AND_BACK, color);
}

void removeNodeHighlight(Node* node) {
    if (!node) return;
    Vec4d color;

    if(node->getName().find("pawn_" + to_string(WHITE)) == 0) color = WHITE_COLOR;
    else if(node->getName().find("pawn_" + to_string(BLACK)) == 0) color = BLACK_COLOR;
    else if(node->getName().find("field_") == 0) color = Vec4d(0.0, 0.0, 0.0, 0.0);

    const ref_ptr stateSet = node->getOrCreateStateSet();
    if (const ref_ptr material = dynamic_cast<Material*>(stateSet->getAttribute(StateAttribute::MATERIAL))) {
        material->setDiffuse(Material::FRONT, color);
    }
}

Vec3d getPawnPosition(const Node* pawn) {
    Matrix pawnPositionMatrix = pawn->asTransform()->asMatrixTransform()->getMatrix();
    return {pawnPositionMatrix(3, 0) - 3.2, pawnPositionMatrix(3, 1), pawnPositionMatrix(3, 2)};
}

bool positionsEquals(const Vec3d& p1, const Vec3d& p2) {
    constexpr double epsilon = 0.001;
    return abs(p1.x() - p2.x()) < epsilon && abs(p1.y() - p2.y()) < epsilon;
}
