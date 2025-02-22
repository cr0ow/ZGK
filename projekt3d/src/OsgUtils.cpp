#include <osg/Node>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osg/ComputeBoundsVisitor>
#include <osgViewer/Viewer>
#include <osg/Material>
#include <memory>
#include <osg/AnimationPath>
#include <osg/Matrix>
#include <osgGA/AnimationPathManipulator>

#include "OsgUtils.h"
#include "Commons.h"

using namespace std;
using namespace osg;
using namespace osgGA;
using osgViewer::Viewer;
using osgDB::readNodeFile;

void animateCamera(Viewer* viewer, const Vec3d& center, const int player, const double distance, const double height) {
    const ref_ptr animationPath = new AnimationPath();
    animationPath->setLoopMode(AnimationPath::NO_LOOPING);

    constexpr int steps = 100;
    const double startAngle = player == WHITE ? DegreesToRadians(WHITE_ANGLE) : DegreesToRadians(BLACK_ANGLE);
    const double endAngle = player == WHITE ? DegreesToRadians(BLACK_ANGLE) : DegreesToRadians(WHITE_ANGLE);

    for(int i = 0; i <= steps; i++) {
        constexpr double duration = 2.0;
        const float t = static_cast<float>(i) / static_cast<float>(steps);
        const double angle = startAngle + t * (endAngle - startAngle);

        Vec3d position(
            center.x() + distance * cos(angle),
            center.y() + distance * sin(angle),
            height
        );

        Vec3d forward = center - position;
        forward.normalize();
        Vec3d side = forward ^ Vec3d(0, 0, 1);
        side.normalize();
        Vec3d up = side ^ forward;

        Matrix viewMatrix(
            side.x(),       side.y(),       side.z(),       0,
            up.x(),         up.y(),         up.z(),         0,
            -forward.x(),   -forward.y(),   -forward.z(),   0,
            position.x(),   position.y(),   position.z(),   1
        );

        animationPath->insert(t * duration, AnimationPath::ControlPoint(position, viewMatrix.getRotate()));
    }

    auto pathManipulator = make_unique<AnimationPathManipulator>(animationPath);
    viewer->setCameraManipulator(pathManipulator.release());
    simulateSpacePress(viewer);
}

void simulateSpacePress(Viewer* viewer) {
    ref_ptr<GUIEventAdapter> event = new osgGA::GUIEventAdapter;
    event->setEventType(GUIEventAdapter::KEYDOWN);
    event->setKey(GUIEventAdapter::KEY_Space);

    viewer->getEventQueue()->addEvent(event);
}

void setCameraPosition(Viewer* viewer, const int player, const bool animated) {
    constexpr double distance = 11.0, height = 13.0;
    const Vec3d center(3.5, 3.5, 0.5);

    if(animated) {
        animateCamera(viewer, center, player, distance, height);
    } else {
        const double angle = player == WHITE ? DegreesToRadians(WHITE_ANGLE) : DegreesToRadians(BLACK_ANGLE);
        const auto eye = Vec3d(
            center.x() + distance * cos(angle),
            center.y() + distance * sin(angle),
            height
        );
        viewer->getCamera()->setViewMatrix(Matrix::lookAt(eye, center, Vec3d(0, 0, 1)));
    }
}

void addPawn(Group* scn, const int player, const int row, const int col) {
    const ref_ptr transform = new MatrixTransform();
    transform->setName("pawn_" + to_string(player) + "_" + to_string(row) + "_" + to_string(col));
    const ref_ptr pawn = readNodeFile("./model/pawn.obj", new osgDB::Options("NoMaterials"));

    if (!pawn) {
        cerr << "Error: unable to load pawn model." << endl;
        exit(EXIT_FAILURE);
    }

    const Vec4d color = player == WHITE ? WHITE_COLOR : BLACK_COLOR;
    const ref_ptr material = new Material;
    material->setDiffuse(Material::FRONT_AND_BACK, color);

    const ref_ptr stateSet = new StateSet();
    stateSet->setAttributeAndModes(material, StateAttribute::ON);
    stateSet->setTextureMode(0, GL_TEXTURE_2D, StateAttribute::OFF);
    pawn->setStateSet(stateSet);

    ComputeBoundsVisitor boundsVisitor;
    pawn->accept(boundsVisitor);
    BoundingBox boundingBox = boundsVisitor.getBoundingBox();
    const float size = boundingBox.xMax() - boundingBox.xMin();
    const float scaleFactor = 1.0f / size;

    transform->setMatrix(
            Matrix::scale(scaleFactor, scaleFactor, scaleFactor) *
            Matrix::rotate(DegreesToRadians(-90.0), Vec3d(1, 0, 0)) *
            Matrix::translate(static_cast<float>(col) + 3.2f, row, 1.87)
    );

    transform->addChild(pawn);
    scn->addChild(transform);
}

ref_ptr<MatrixTransform> createField(const int row, const int col, const bool isWhite) {
    const ref_ptr field = new Box(Vec3(0, 0, 0), 1.0f, 1.0f, 0.3f);
    const ref_ptr drawable = new ShapeDrawable(field);

    const Vec4 color = isWhite ? Vec4(1.0f, 1.0f, 1.0f, 1.0f) : Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    drawable->setColor(color);

    const ref_ptr geode = new Geode();
    geode->addDrawable(drawable);

    ref_ptr transform = new MatrixTransform();
    transform->setName("field_" + to_string(row) + "_" + to_string(col));
    transform->setMatrix(Matrix::translate(col, row, 0));
    transform->addChild(geode);

    return transform;
}

void addBoard(Group* scn) {
    const ref_ptr board = new Group();
    board->setName("board");
    scn->addChild(board);

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            const bool isWhite = (row + col) % 2 == 0;
            board->addChild(createField(row, col, isWhite));

            if(!isWhite && (row < 2 || row > 5)) {
                const int color = row < 2 ? BLACK : WHITE;
                addPawn(scn, color, row, col);
            }
        }
    }
}
