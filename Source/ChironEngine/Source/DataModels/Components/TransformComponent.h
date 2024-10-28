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

    // ------------- GETTERS ----------------------

    inline const Vector3& GetLocalPos() const;
    inline const Vector3& GetLocalRotXYZ() const;
    inline const Vector3& GetLocalSca() const;

    // ------------- SETTERS ----------------------

    inline void SetLocalPos(const Vector3& localPos);
    inline void SetLocalRot(const Vector3& localRotV);
    inline void SetLocalSca(const Vector3& localSca);
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

inline const Vector3& TransformComponent::GetLocalPos() const
{
    return _localPos;
}

inline const Vector3& TransformComponent::GetLocalRotXYZ() const
{
    return _rotXYZ;
}

inline const Vector3& TransformComponent::GetLocalSca() const
{
    return _localSca;
}

inline void TransformComponent::SetLocalPos(const Vector3& localPos)
{
    _localPos = localPos;
}

inline void TransformComponent::SetLocalRot(const Vector3& localRotV)
{
    _rotXYZ = localRotV;
    _localRot = Quaternion::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(_rotXYZ.y), 
        DirectX::XMConvertToRadians(_rotXYZ.x), DirectX::XMConvertToRadians(_rotXYZ.z));
}

inline void TransformComponent::SetLocalSca(const Vector3& localSca)
{
    _localSca = localSca;
}
