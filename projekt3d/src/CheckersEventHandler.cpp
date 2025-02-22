#include <osg/Node>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osg/ComputeBoundsVisitor>
#include <osgViewer/Viewer>
#include <osgGA/GUIEventHandler>
#include <osgUtil/LineSegmentIntersector>
#include <osg/Material>
#include <iostream>
#include <memory>
#include <thread>
#include <osg/AnimationPath>
#include <osg/Matrix>
#include <osgGA/AnimationPathManipulator>
#include "CheckersEventHandler.h"
#include "CheckersUtils.h"
#include "OsgUtils.h"
#include "PawnAnimationCallback.h"

bool CheckersEventHandler::handle(const GUIEventAdapter& ea, GUIActionAdapter& aa) {
    if(!dynamic_cast<osgViewer::View*>(&aa)) return false;

    auto* view = dynamic_cast<osgViewer::View*>(&aa);

    if (ea.getEventType() == GUIEventAdapter::PUSH && ea.getButton() == GUIEventAdapter::LEFT_MOUSE_BUTTON) {
        if (osgUtil::LineSegmentIntersector::Intersections intersections; view->computeIntersections(ea.getX(), ea.getY(), intersections)) {
            for (auto& hit : intersections) {
                NodePath path = hit.nodePath;
                for (Node* node : path) {
                    if (node->getName().find("pawn_" + to_string(_player)) == 0) {
                        _selectedPawn = node;
                        removeHighlights();
                        highlightSelectedPawn();
                        highlightReachableFields();
                        return true;
                    }
                    if (node->getName().find("field_") == 0 && isFieldValidToMoveOn(node)) {
                        if (_selectedPawn) {
                            movePawn(_selectedPawn, node);
                            removeHighlights();
                            _selectedPawn = nullptr;
                        }
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void CheckersEventHandler::removePawn(const Vec3d &position) const {
    for(int i = 1; i < _scene->getNumChildren(); i++) {
        if(positionsEquals(_scene->getChild(i)->asTransform()->asMatrixTransform()->getMatrix().getTrans(), position)) {
            _scene->removeChild(i);
            return;
        }
    }
}

void CheckersEventHandler::highlightReachableFields() const {
    Group* board = _scene->getChild(0)->asGroup();
    for(int i = 0; i < board->getNumChildren(); i++) {
        if (isFieldValidToMoveOn(board->getChild(i))) {
            addHighlight(board->getChild(i), Vec4(1.0f, 1.0f, 0.0f, 1.0f));
        }
    }
}

void CheckersEventHandler::highlightSelectedPawn() const {
    if (!_selectedPawn) return;
    addHighlight(_selectedPawn, Vec4(1.0f, 0.5f, 0.0f, 1.0f));
}

void CheckersEventHandler::removeHighlights() const {
    Group* board = _scene->getChild(0)->asGroup();
    for(int i = 0; i < board->getNumChildren(); i++) {
        removeNodeHighlight(board->getChild(i));
    }
    for(int i = 1; i < _scene->getNumChildren(); i++) {
        removeNodeHighlight(_scene->getChild(i));
    }
}

bool CheckersEventHandler::isJumpPossible(const Vec3d &fieldPosition, const Vec3d &pawnPosition, const Vec3d &jumpDirection) const {
    if(positionsEquals(pawnPosition + (jumpDirection * 2), fieldPosition)) {
        for(int i = 1; i < _scene->getNumChildren(); i++) {
            if(const int player = _player == WHITE ? BLACK : WHITE;
                _scene->getChild(i)->getName().find("pawn_" + to_string(player)) == 0 &&
                positionsEquals(pawnPosition + jumpDirection, getPawnPosition(_scene->getChild(i)))) {
                return true;
            }
        }
    }
    return false;
}

bool CheckersEventHandler::isFieldValidToMoveOn(const Node* node) const {
    if (!_selectedPawn) return false;
    const int row = stoi(node->getName().substr(6, 1));
    const int col = stoi(node->getName().substr(8, 1));

    if ((row + col) % 2 == 0) {
        return false;
    }

    Vec3d pawnPosition = getPawnPosition(_selectedPawn);
    const Vec3d fieldPosition(col, row, pawnPosition.z());

    for(int i = 1; i < _scene->getNumChildren(); i++) {
        if(positionsEquals(getPawnPosition(_scene->getChild(i)), fieldPosition)) {
            return false;
        }
    }

    if(_player == BLACK) {
        return isJumpPossible(fieldPosition, pawnPosition, Vec3d(-1.0, 1.0, 0.0)) ||
               isJumpPossible(fieldPosition, pawnPosition, Vec3d(1.0, 1.0, 0.0)) ||
               positionsEquals(pawnPosition + Vec3d(-1.0, 1.0, 0.0), fieldPosition) ||
               positionsEquals(pawnPosition + Vec3d(1.0, 1.0, 0.0), fieldPosition);
    }
    return isJumpPossible(fieldPosition, pawnPosition, Vec3d(-1.0, -1.0, 0.0)) ||
           isJumpPossible(fieldPosition, pawnPosition, Vec3d(1.0, -1.0, 0.0)) ||
           positionsEquals(pawnPosition + Vec3d(-1.0, -1.0, 0.0), fieldPosition) ||
           positionsEquals(pawnPosition + Vec3d(1.0, -1.0, 0.0), fieldPosition);
}

void CheckersEventHandler::movePawn(Node* pawn, Node* field) {
    Vec3d jumpedPawnPosition;
    const auto* fieldTransform = dynamic_cast<MatrixTransform*>(field->asTransform());
    if (!fieldTransform) {
        cerr << "Error: Field transform is null." << endl;
        return;
    }

    const Matrix currentMatrix = pawn->asTransform()->asMatrixTransform()->getMatrix();
    Vec3d startPosition = currentMatrix.getTrans();
    Vec3d fieldPosition = fieldTransform->getMatrix().getTrans();
    const auto targetPosition = Vec3d(fieldPosition.x() + 3.2, fieldPosition.y(), 1.87);
    Vec3d moveVector = targetPosition - startPosition;

    const Matrix rotationAndScale = Matrix::scale(currentMatrix.getScale()) *
                                    Matrix::rotate(DegreesToRadians(-90.0), Vec3d(1, 0, 0));

    float duration = 0.5;
    float height = 1.0;
    if(abs(startPosition.y() - fieldPosition.y()) > 1.0) {
        duration = 0.7;
        height = 2.4;
        jumpedPawnPosition = Vec3d((startPosition.x() + targetPosition.x()) / 2, (startPosition.y() + targetPosition.y()) / 2, 1.87);
    }

    const ref_ptr animationPath = new AnimationPath();
    animationPath->setLoopMode(AnimationPath::NO_LOOPING);

    constexpr int steps = 50;
    for(int i = 0; i <= steps; i++) {
        const float t = static_cast<float>(i) / static_cast<float>(steps);
        const double angle = DegreesToRadians(180.0 * t);
        const double z = height * sin(angle);
        animationPath->insert(t * duration, AnimationPath::ControlPoint(startPosition + Vec3d(moveVector.x() * t, moveVector.y() * t, z)));
    }

    auto onAnimationEnd = [this] {
        setCameraPosition(_viewer, _player, true);
        _player = (_player == WHITE) ? BLACK : WHITE;
    };

    const ref_ptr animationCallback = new PawnAnimationCallback(animationPath, rotationAndScale, duration, onAnimationEnd, jumpedPawnPosition, this);
    pawn->setUpdateCallback(animationCallback);
}