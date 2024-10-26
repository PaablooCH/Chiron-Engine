#pragma once
#include "Component.h"

class TransformComponent : public Component
{
public:
    TransformComponent(GameObject* owner);
    TransformComponent(const TransformComponent& copy);
    ~TransformComponent() override;

    void UpdateMatrices();
    void CalculateLocalFromNewGlobal(const TransformComponent* newTransformFrom);

private:
    void RecalculateMatrices();

private:
    Vector3 _localPos;
    Quaternion _localRot;
    Vector3 _localSca;
    Matrix _localMatrix;

    Vector3 _globalPos;
    Quaternion _globalRot;
    Vector3 _globalSca;
    Matrix _globalMatrix;

    Vector3 _bbPos;
    Vector3 _bbSca;
    Vector3 _originScaling;
    Vector3 _originCenter;

    Vector3 _rotXYZ;

    DirectX::BoundingBox _localAABB;
    DirectX::BoundingBox _encapsuledAABB;
    DirectX::BoundingOrientedBox _objectOBB;
    
    bool _drawBoundingBoxes;
    bool _uniformScale;
};

