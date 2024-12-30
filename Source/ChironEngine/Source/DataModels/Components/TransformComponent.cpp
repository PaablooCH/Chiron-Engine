#include "Pch.h"
#include "TransformComponent.h"

#include "DataModels/GameObject/GameObject.h"

TransformComponent::TransformComponent(GameObject* owner) :
    Component(ComponentType::TRANSFORM, owner),
    _localPos(Vector3::Zero),
    _localRot(Quaternion::Identity),
    _localSca(Vector3::One),
    _localMatrix(Matrix::Identity),
    _globalPos(Vector3::Zero),
    _globalRot(Quaternion::Identity),
    _globalSca(Vector3::One),
    _globalMatrix(Matrix::Identity),
    _bbPos(Vector3::Zero),
    _bbSca(Vector3::One),
    _originScaling(Vector3(0.5f, 0.5f, 0.5f)),
    _originCenter(Vector3(0.5f, 0.5f, 0.5f)),
    _rotXYZ(Vector3::Zero),
    _localAABB({ { 0, 0, 0 }, { 1, 1, 1 } }),
    _encapsuledAABB(_localAABB),
    _objectOBB({ { 0, 0, 0 }, { 1, 1, 1 }, Vector4::UnitW }),
    _drawBoundingBoxes(false),
    _uniformScale(true)
{
    RecalculateMatrices();
}

TransformComponent::TransformComponent(const TransformComponent& copy) :
    Component(copy),
    _localPos(copy._localPos),
    _localRot(copy._localRot),
    _localSca(copy._localSca),
    _localMatrix(copy._localMatrix),
    _globalPos(copy._globalPos),
    _globalRot(copy._globalRot),
    _globalSca(copy._globalSca),
    _globalMatrix(copy._globalMatrix),
    _bbPos(copy._bbPos),
    _bbSca(copy._bbSca),
    _originScaling(copy._originScaling),
    _originCenter(copy._originCenter),
    _rotXYZ(copy._rotXYZ),
    _localAABB(copy._localAABB),
    _encapsuledAABB(copy._encapsuledAABB),
    _objectOBB(copy._objectOBB),
    _drawBoundingBoxes(copy._drawBoundingBoxes),
    _uniformScale(copy._uniformScale)
{
    RecalculateMatrices();
}

TransformComponent::~TransformComponent()
{
}

void TransformComponent::UpdateMatrices(bool notify /* = true */)
{
    RecalculateMatrices();
    if (notify)
    {
        for (auto component : _owner->GetComponents())
        {
            component->NotifyTransform();
        }
    }
    for (auto child : _owner->GetChildren())
    {
        child->GetInternalComponent<TransformComponent>()->UpdateMatrices();
    }
}

void TransformComponent::CalculateLocalFromNewGlobal(const TransformComponent* newTransformFrom)
{
    _localMatrix = newTransformFrom->_globalMatrix.Invert() * _globalMatrix;
    _localMatrix.Decompose(_localSca, _localRot, _localPos);
    Vector3 euler = _localRot.ToEuler();
    _rotXYZ.x = DirectX::XMConvertToDegrees(euler.x);
    _rotXYZ.y = DirectX::XMConvertToDegrees(euler.y);
    _rotXYZ.z = DirectX::XMConvertToDegrees(euler.z);
}

void TransformComponent::InternalSave(Field& meta)
{
    meta["localPos_X"] = _localPos.x;
    meta["localPos_Y"] = _localPos.y;
    meta["localPos_Z"] = _localPos.z;

    meta["localRot_X"] = _rotXYZ.x;
    meta["localRot_Y"] = _rotXYZ.y;
    meta["localRot_Z"] = _rotXYZ.z;

    meta["localSca_X"] = _localSca.x;
    meta["localSca_Y"] = _localSca.y;
    meta["localSca_Z"] = _localSca.z;
}

void TransformComponent::InternalLoad(const Field& meta)
{
    _localPos.x = meta["localPos_X"];
    _localPos.y = meta["localPos_Y"];
    _localPos.z = meta["localPos_Z"];

    Vector3 rot;
    rot.x = meta["localRot_X"];
    rot.y = meta["localRot_Y"];
    rot.z = meta["localRot_Z"];
    SetLocalRot(rot);

    _localSca.x = meta["localSca_X"];
    _localSca.y = meta["localSca_Y"];
    _localSca.z = meta["localSca_Z"];

    RecalculateMatrices();
}

void TransformComponent::RecalculateMatrices()
{
    _localMatrix = Matrix::CreateScale(_localSca) * Matrix::CreateFromQuaternion(_localRot) * Matrix::CreateTranslation(_localPos);

    auto parent = _owner->GetParent();
    if (parent)
    {
        auto parentGlobalMatrix = parent->GetInternalComponent<TransformComponent>()->_globalMatrix;
        _globalMatrix = parentGlobalMatrix * _localMatrix;
    }
    else
    {
        _globalMatrix = _localMatrix;
    }
    _globalMatrix.Decompose(_globalSca, _globalRot, _globalPos);
}