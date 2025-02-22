#ifndef PAWNANIMATIONCALLBACK_H
#define PAWNANIMATIONCALLBACK_H

#include <osg/Node>
#include <osg/Material>
#include <osg/Matrix>
#include <osg/AnimationPath>
#include "CheckersEventHandler.h"

using namespace std;
using namespace osg;

class PawnAnimationCallback : public AnimationPathCallback {
public:
    PawnAnimationCallback(AnimationPath* path, const Matrix& scaleAndRotation,
        const float animationDuration, const function<void()> &onAnimationEnd, const Vec3d& pawnToRemove,
        CheckersEventHandler* superHandler) : AnimationPathCallback(path),
        _scaleAndRotation(scaleAndRotation),
        _animationDuration(animationDuration),
        _startTime(Timer::instance()->tick()),
        _onAnimationEnd(onAnimationEnd),
        _pawnToRemove(pawnToRemove),
        _super(superHandler) {}

    void operator()(Node* node, NodeVisitor* nv) override {
        if (auto* transform = dynamic_cast<MatrixTransform*>(node)) {
            AnimationPathCallback::operator()(node, nv);
            const Matrix animationMatrix = transform->getMatrix();
            transform->setMatrix(_scaleAndRotation * animationMatrix);

            if (const double elapsedTime = Timer::instance()->delta_s(_startTime, Timer::instance()->tick());
                elapsedTime >= _animationDuration + 0.2) {
                if (_onAnimationEnd) {
                    _super->removePawn(_pawnToRemove);
                    _onAnimationEnd();
                    _onAnimationEnd = nullptr;
                }
            }
        }
    }

private:
    Matrix _scaleAndRotation;
    float _animationDuration;
    Timer_t _startTime;
    function<void()> _onAnimationEnd;
    Vec3d _pawnToRemove;
    CheckersEventHandler* _super;
};

#endif //PAWNANIMATIONCALLBACK_H
