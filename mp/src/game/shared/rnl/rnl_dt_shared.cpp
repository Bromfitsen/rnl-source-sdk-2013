//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "rnl_dt_shared.h"

#ifdef CLIENT_DLL
#include "recvproxy.h"
#else
#include "sendproxy.h"
#endif

DataTableProp PropBool(
	char* pVarName,
	int offset,
	int sizeofVar)
{

#ifdef CLIENT_DLL
	return RecvPropBool(pVarName, offset, sizeofVar);
#else
	return SendPropBool(pVarName, offset, sizeofVar);
#endif
}

DataTableProp PropDataTable(
	const char* pVarName,
	int offset,
	int flags,
	NetDataTable* pTable,
	DataTableProxyFunc varProxy
)
{
#if !defined (CLIENT_DLL)
	return SendPropDataTable(pVarName, offset, pTable, varProxy);
#else
	return RecvPropDataTable(pVarName, offset, flags, pTable, varProxy);
#endif
}

DataTableProp PropArray(
	const char* pVarName,
	int offset,
	int sizeofVar,
	int elements,
	DataTableProp pArrayProp,
	DataTableProxyFunc varProxy
)
{
#if !defined (CLIENT_DLL)
	return SendPropArray3(pVarName, offset, sizeofVar, elements, pArrayProp, varProxy);
#else
	return RecvPropArray3(pVarName, offset, sizeofVar, elements, pArrayProp, varProxy);
#endif
}

DataTableProp PropUtlVector(
	char* pVarName,
	int offset,
	int sizeofVar,
#ifdef CLIENT_DLL
	ResizeUtlVectorFn resizeFn,
#endif
	EnsureCapacityFn ensureFn,

	int nMaxElements,
	DataTableProp pArrayProp
#ifndef CLIENT_DLL
	, DataTableProxyFunc varProxy
#endif
)
{
#if !defined (CLIENT_DLL)
	return SendPropUtlVector(pVarName, offset, sizeofVar, ensureFn, nMaxElements, pArrayProp, varProxy);
#else
	return RecvPropUtlVector(pVarName, offset, sizeofVar, resizeFn, ensureFn, nMaxElements, pArrayProp);
#endif
}

DataTableProp PropInt(
	char* pVarName,
	int offset,
	int sizeofVar,	// Handled by SENDINFO macro.
	int nBits,					// Set to -1 to automatically pick (max) number of bits based on size of element.
	int flags,
	PropVarProxyFn varProxy

)
{
#if !defined (CLIENT_DLL)
	return SendPropInt(pVarName, offset, sizeofVar, nBits, flags, varProxy);
#else
	return RecvPropInt(pVarName, offset, sizeofVar, flags, varProxy);
#endif
}


DataTableProp PropTime(
	const char* pVarName,
	int offset,
	int sizeofVar
)
{
#if !defined (CLIENT_DLL)
	return SendPropTime(pVarName, offset, sizeofVar);
#else
	return RecvPropTime(pVarName, offset, sizeofVar);
#endif
}

DataTableProp PropModelIndex(
	const char* pVarName,
	int offset,
	int sizeofVar
)
{
#if !defined (CLIENT_DLL)
	return SendPropModelIndex(pVarName, offset, sizeofVar);
#else
	return RecvPropInt(pVarName, offset, sizeofVar);
#endif
}