#ifndef CHECKERSEVENTHANDLER_H
#define CHECKERSEVENTHANDLER_H

#include <osg/Node>
#include <osgViewer/Viewer>
#include "Commons.h"

using namespace std;
using namespace osg;
using namespace osgGA;
using osgViewer::Viewer;

class CheckersEventHandler final : public GUIEventHandler {
public:
    CheckersEventHandler(Viewer* viewer, const ref_ptr<Group>& scene)
        : _viewer(viewer), _scene(scene), _selectedPawn(nullptr), _lastHoveredNode(nullptr) {}

    bool handle(const GUIEventAdapter& ea, GUIActionAdapter& aa) override;
	void removePawn(const Vec3d& position) const;

private:
    void highlightReachableFields() const;
    void highlightSelectedPawn() const;
    void removeHighlights() const;
    bool isJumpPossible(const Vec3d &fieldPosition, const Vec3d &pawnPosition, const Vec3d &jumpDirection) const;
    bool isFieldValidToMoveOn(const Node* node) const;
    void movePawn(Node* pawn, Node* field);

    Viewer* _viewer;
    Group* _scene;
    Node* _selectedPawn;
    Node* _lastHoveredNode;
    Vec4d _originalColor;
    int _player = WHITE;
};

#endif //CHECKERSEVENTHANDLER_H
