//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef RNL_DATATABLE_SHARED_H
#define RNL_DATATABLE_SHARED_H

#ifdef _WIN32
#pragma once
#endif

#include "dt_shared.h"

// ------------------------------------------------------------------------ //
// Client version
// ------------------------------------------------------------------------ //

#if defined (CLIENT_DLL)

#include "dt_utlvector_recv.h"

#define PROPINFO_ARRAY(varName)					RECVINFO(varName[0])
#define PROPINFO_ARRAY3(varName)				RECVINFO_ARRAY(varName)
#define PROPINFO_UTLVECTOR(varName)				RECVINFO_UTLVECTOR(varName)
#define PROPINFO_UTLVECTOR_RESIZE(varName, resizeFn)		RECVINFO_UTLVECTOR_SIZEFN(varName, resizeFn)

#define EXTERN_PROP_TABLE(tableName)			EXTERN_RECV_TABLE(tableName)
#define REFERENCE_PROP_TABLE(tableName)			REFERENCE_RECV_TABLE(tableName)

#define DataTableProxyFunc DataTableRecvVarProxyFn
#define NetDataTable  RecvTable

#define PropVarProxyFn RecvVarProxyFn

#define DataTableProxy_TableToTable		DataTableRecvProxy_StaticDataTable

#endif // CLIENT_DLL


// ------------------------------------------------------------------------ //
// Server version
// ------------------------------------------------------------------------ //

#if !defined (CLIENT_DLL)

#include "dt_utlvector_send.h"

#ifndef BEGIN_PREDICTION_DATA_NO_BASE
#define BEGIN_PREDICTION_DATA_NO_BASE( className ) BEGIN_PREDICTION_DATA( className )
#endif

#define PROPINFO_ARRAY(varName)					SENDINFO_ARRAY(varName)
#define PROPINFO_ARRAY3(varName)				SENDINFO_ARRAY3(varName)
#define PROPINFO_UTLVECTOR(varName)				SENDINFO_UTLVECTOR(varName)		

#define EXTERN_PROP_TABLE(tableName)			EXTERN_SEND_TABLE(tableName)
#define REFERENCE_PROP_TABLE(tableName)			REFERENCE_SEND_TABLE(tableName)

#define DataTableProxyFunc SendTableProxyFn
#define NetDataTable SendTable

#define PropVarProxyFn SendVarProxyFn

#define DataTableProxy_TableToTable		SendProxy_DataTableToDataTable

#endif // !CLIENT_DLL

DataTableProp PropBool(
	char* pVarName,
	int offset,
	int sizeofVar);

DataTableProp PropDataTable(
	const char* pVarName,
	int offset,
	int flags,
	NetDataTable* pTable,
	DataTableProxyFunc varProxy = DataTableProxy_TableToTable
);


DataTableProp PropArray(
	const char* pVarName,
	int offset,
	int sizeofVar,
	int elements,
	DataTableProp pArrayProp,
	DataTableProxyFunc varProxy = DataTableProxy_TableToTable
);

DataTableProp PropUtlVector(
	char* pVarName,				// Use NETPROPINFO_UTLVECTOR to generate these first 4 parameters.
	int offset,
	int sizeofVar,
#ifdef CLIENT_DLL
	ResizeUtlVectorFn resizeFn,
#endif
	EnsureCapacityFn ensureFn,

	int nMaxElements,			// Max # of elements in the array. Keep this as low as possible.
	// Describe the data inside of each element in the array.
	DataTableProp pArrayProp
#ifndef CLIENT_DLL
	, DataTableProxyFunc varProxy = DataTableProxy_TableToTable
#endif
);

#define PropUtlVectorDataTable( varName, nMaxElements, dataTableName ) \
	PropUtlVector( \
		PROPINFO_UTLVECTOR( varName ), \
		nMaxElements, \
		PropDataTable( NULL, 0, 0, &REFERENCE_PROP_TABLE( dataTableName ) ) \
		)

#define PropUtlVectorDataTable_Resize( varName, resizeFunc, nMaxElements, dataTableName ) \
	PropUtlVector( \
		PROPINFO_UTLVECTOR_RESIZE( varName, resizeFunc ), \
		nMaxElements, \
		PropDataTable( NULL, 0, 0, &REFERENCE_PROP_TABLE( dataTableName ) ) \
		)


DataTableProp PropInt(
	char* pVarName,
	int offset,
	int sizeofVar,	// Handled by SENDINFO macro.
	int nBits,					// Set to -1 to automatically pick (max) number of bits based on size of element.
	int flags,
	PropVarProxyFn varProxy
);

DataTableProp PropTime(
	const char* pVarName,
	int offset,
	int sizeofVar = SIZEOF_IGNORE);

DataTableProp PropModelIndex(
	const char* pVarName,
	int offset,
	int sizeofVar = SIZEOF_IGNORE
);

#ifdef CLIENT_DLL
#define PropArray2(arrayLengthSendProxy, varTemplate, elementCount, elementStride, arrayName) \
	RecvPropArray2(arrayLengthSendProxy, varTemplate, elementCount, elementStride, arrayName)
#else
#define PropArray2(arrayLengthSendProxy, varTemplate, elementCount, elementStride, arrayName) \
	SendPropArray2(arrayLengthSendProxy, varTemplate, elementCount, elementStride, arrayName)
#endif

#endif // RNL_DATATABLE_SHARED_H

