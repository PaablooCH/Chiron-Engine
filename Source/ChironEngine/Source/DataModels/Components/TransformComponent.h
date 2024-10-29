#pragma once
#include "Component.h"

enum class Axis
{
    X,
    Y,
    Z,
    NONE
};

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
    
    inline const bool GetUniformScale() const;

    // ------------- SETTERS ----------------------

    inline void SetLocalPos(const Vector3& pos);
    inline void SetLocalRot(const Vector3& rotXYZ);
    inline void SetLocalSca(const Vector3& scale);
    inline void SetScaleUniform(const Vector3& localSca, Axis axis);


    inline void SetUniformScale(bool uniformScale);

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

inline const bool TransformComponent::GetUniformScale() const
{
    return _uniformScale;
}

inline void TransformComponent::SetLocalPos(const Vector3& pos)
{
    _localPos = pos;
}

inline void TransformComponent::SetLocalRot(const Vector3& rotXYZ)
{
    _rotXYZ = rotXYZ;
    _localRot = Quaternion::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(_rotXYZ.y), 
        DirectX::XMConvertToRadians(_rotXYZ.x), DirectX::XMConvertToRadians(_rotXYZ.z));
}

inline void TransformComponent::SetLocalSca(const Vector3& scale)
{
    _localSca.x = std::max(scale.x, 0.001f);
    _localSca.y = std::max(scale.y, 0.001f);
    _localSca.z = std::max(scale.z, 0.001f);
}

inline void TransformComponent::SetScaleUniform(const Vector3& scale, Axis modifiedAxis)
{
    switch (modifiedAxis)
    {
    case Axis::X:
        _localSca.y = std::max(scale.y * scale.x / _localSca.x, 0.001f);
        _localSca.z = std::max(scale.z * scale.x / _localSca.x, 0.001f);
        _localSca.x = std::max(scale.x, 0.001f);
        break;
    case Axis::Y:
        _localSca.x = std::max(scale.x * scale.y / _localSca.y, 0.001f);
        _localSca.z = std::max(scale.z * scale.y / _localSca.y, 0.001f);
        _localSca.y = std::max(scale.y, 0.001f);
        break;
    case Axis::Z:
        _localSca.x = std::max(scale.x * scale.z / _localSca.z, 0.001f);
        _localSca.y = std::max(scale.y * scale.z / _localSca.z, 0.001f);
        _localSca.z = std::max(scale.z, 0.001f);
        break;
    case Axis::NONE:
        LOG_ERROR("Axis not specified");
        break;
    }
}

inline void TransformComponent::SetUniformScale(bool uniformScale)
{
    _uniformScale = uniformScale;
}
