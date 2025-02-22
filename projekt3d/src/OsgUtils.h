#ifndef OSGUTILS_H
#define OSGUTILS_H

#include <osg/Group>
#include <osgViewer/Viewer>

using namespace osg;
using osgViewer::Viewer;

void animateCamera(Viewer* viewer, const Vec3d& center, const int player, const double distance, const double height);
void simulateSpacePress(Viewer* viewer);
void setCameraPosition(Viewer* viewer, const int player, const bool animated);
void addPawn(Group* scn, const int player, const int row, const int col);
ref_ptr<MatrixTransform> createField(const int row, const int col, const bool isWhite);
void addBoard(Group* scn);

#endif
