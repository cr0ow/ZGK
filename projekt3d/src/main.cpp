#include <osg/Group>
#include <osgViewer/Viewer>
#include <osgGA/AnimationPathManipulator>

#include "CheckersEventHandler.h"
#include "OsgUtils.h"
#include "Commons.h"

using namespace osg;
using osgViewer::Viewer;

int main() {
    const ref_ptr scene = new Group();
    const ref_ptr viewer = new Viewer();

    addBoard(scene);

    viewer->getCamera()->setAllowEventFocus(false);
    setCameraPosition(viewer, WHITE, false);

    viewer->setUpViewInWindow(100, 100, 800, 600);
    viewer->setCameraManipulator(nullptr);
    viewer->setSceneData(scene);

    viewer->addEventHandler(new CheckersEventHandler(viewer, scene));

    return viewer->run();
}
