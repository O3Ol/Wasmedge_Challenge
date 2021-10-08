#include <wasmedge/wasmedge.h>
#include <stdio.h>

/* Host function body definition. */
WasmEdge_Result Add(void *Data, WasmEdge_MemoryInstanceContext *MemCxt,
                    const WasmEdge_Value *In, WasmEdge_Value *Out) {
  int32_t Val1 = WasmEdge_ValueGetI32(In[0]);
  int32_t Val2 = WasmEdge_ValueGetI32(In[1]);
  printf("Host function \"Add\": %d + %d\n", Val1, Val2);
  Out[0] = WasmEdge_ValueGenI32(Val1 + Val2);
  /* Also set the result to the data. */
  int32_t *DataPtr = (int32_t *)Data;
  *DataPtr = Val1 + Val2;
  return WasmEdge_Result_Success;
}

int main() {
  /* Create the VM context. */
  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(NULL, NULL);

  /* The WASM module buffer. */
  uint8_t WASM[] = {
    /* WASM header */
    0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00,
    /* Type section */
    0x01, 0x07, 0x01,
    /* function type {i32, i32} -> {i32} */
    0x60, 0x02, 0x7F, 0x7F, 0x01, 0x7F,
    /* Import section */
    0x02, 0x13, 0x01,
    /* module name: "extern" */
    0x06, 0x65, 0x78, 0x74, 0x65, 0x72, 0x6E,
    /* extern name: "func-add" */
    0x08, 0x66, 0x75, 0x6E, 0x63, 0x2D, 0x61, 0x64, 0x64,
    /* import desc: func 0 */
    0x00, 0x00,
    /* Function section */
    0x03, 0x02, 0x01, 0x00,
    /* Export section */
    0x07, 0x0A, 0x01,
    /* export name: "addTwo" */
    0x06, 0x61, 0x64, 0x64, 0x54, 0x77, 0x6F,
    /* export desc: func 0 */
    0x00, 0x01,
    /* Code section */
    0x0A, 0x0A, 0x01,
    /* code body */
    0x08, 0x00, 0x20, 0x00, 0x20, 0x01, 0x10, 0x00, 0x0B
  };

  /* The external data object: an integer. */
  int32_t Data;

  /* Create the import object. */
  WasmEdge_String ExportName = WasmEdge_StringCreateByCString("extern");
  WasmEdge_ImportObjectContext *ImpObj = WasmEdge_ImportObjectCreate(ExportName);
  enum WasmEdge_ValType ParamList[2] = { WasmEdge_ValType_I32, WasmEdge_ValType_I32 };
  enum WasmEdge_ValType ReturnList[1] = { WasmEdge_ValType_I32 };
  WasmEdge_FunctionTypeContext *HostFType = WasmEdge_FunctionTypeCreate(ParamList, 2, ReturnList, 1);
  WasmEdge_HostFunctionContext *HostFunc = WasmEdge_HostFunctionCreate(HostFType, Add, &Data, 0);
  WasmEdge_FunctionTypeDelete(HostFType);
  WasmEdge_String HostFuncName = WasmEdge_StringCreateByCString("func-add");
  WasmEdge_ImportObjectAddHostFunction(ImpObj, HostFuncName, HostFunc);
  WasmEdge_StringDelete(HostFuncName);

  WasmEdge_VMRegisterModuleFromImport(VMCxt, ImpObj);

  /* The parameters and returns arrays. */
  WasmEdge_Value Params[2] = { WasmEdge_ValueGenI32(1234), WasmEdge_ValueGenI32(5678) };
  WasmEdge_Value Returns[1];
  /* Function name. */
  WasmEdge_String FuncName = WasmEdge_StringCreateByCString("addTwo");
  /* Run the WASM function from file. */
  WasmEdge_Result Res = WasmEdge_VMRunWasmFromBuffer(
    VMCxt, WASM, sizeof(WASM), FuncName, Params, 2, Returns, 1);

  if (WasmEdge_ResultOK(Res)) {
    printf("Get the result: %d\n", WasmEdge_ValueGetI32(Returns[0]));
  } else {
    printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
  }
  printf("Data value: %d\n", Data);

  /* Resources deallocations. */
  WasmEdge_VMDelete(VMCxt);
  WasmEdge_StringDelete(FuncName);
  WasmEdge_ImportObjectDelete(ImpObj);
  return 0;
}
