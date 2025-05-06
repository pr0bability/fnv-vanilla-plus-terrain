#pragma once

#include "DirectXMath.h"

class NiPoint2;
class NiPoint3;
class NiPoint4;
class NiColor;
class NiColorA;
class NiMatrix3;
class NiBound;
class NiTransform;
class NiPlane;
class NiQuaternion;

namespace DirectX {

	inline XMMATRIX XM_CALLCONV XMLoadD3DXMATRIX(const D3DXMATRIX& arSource) noexcept {
		return XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&arSource));
	}

	inline void XM_CALLCONV XMStoreD3DXMATRIX(D3DXMATRIX& arDest, const XMMATRIX& arSource) noexcept {
		XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&arDest), arSource);
	}

	inline XMVECTOR XM_CALLCONV XMLoadD3DXVECTOR3(const D3DXVECTOR3& arSource) noexcept {
		return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&arSource));
	}

	inline void XM_CALLCONV XMStoreD3DXVECTOR3(D3DXVECTOR3& arDest, const XMVECTOR& arSource) noexcept {
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&arDest), arSource);
	}

	inline XMVECTOR XM_CALLCONV XMLoadD3DXVECTOR4(const D3DXVECTOR4& arSource) noexcept {
		return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&arSource));
	}

	inline void XM_CALLCONV XMStoreD3DXVECTOR4(D3DXVECTOR4& arDest, const XMVECTOR& arSource) noexcept {
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&arDest), arSource);
	}

	inline XMVECTOR XM_CALLCONV XMLoadNiPoint3(const NiPoint3& arSource) noexcept {
		return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&arSource));
	}

	inline void XM_CALLCONV XMStoreNiPoint3(NiPoint3& arDest, const XMVECTOR& arSource) noexcept {
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&arDest), arSource);
	}

	inline XMVECTOR XM_CALLCONV XMLoadNiPoint4(const NiPoint4& arSource) noexcept {
		return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&arSource));
	}

	inline void XM_CALLCONV XMStoreNiPoint4(NiPoint4& arDest, const XMVECTOR& arSource) noexcept {
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&arDest), arSource);
	}

	inline XMVECTOR XM_CALLCONV XMLoadNiBound(const NiBound& arSource) noexcept {
		return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&arSource));
	}

	inline XMVECTOR XM_CALLCONV XMLoadNiColor(const NiColor& arSource) noexcept {
		return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&arSource));
	}

	inline void XM_CALLCONV XMStoreNiColor(NiColor& arDest, const XMVECTOR& arSource) noexcept {
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&arDest), arSource);
	}

	inline XMVECTOR XM_CALLCONV XMLoadNiColorA(const NiColorA& arSource) noexcept {
		return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&arSource));
	}

	inline void XM_CALLCONV XMStoreNiColorA(NiColorA& arDest, const XMVECTOR& arSource) noexcept {
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&arDest), arSource);
	}

}