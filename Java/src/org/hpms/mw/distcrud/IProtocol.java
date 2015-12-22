package org.hpms.mw.distcrud;

interface IProtocol {

   byte[] SIGNATURE = {'D','C','R','U','D'};

   String ICRUD_INTERFACE_NAME    = "dcrud.ICRUD";
   String ICRUD_INTERFACE_CREATE  = "create";
   String ICRUD_INTERFACE_UPDATE  = "update";
   String ICRUD_INTERFACE_DELETE  = "delete";
   String ICRUD_INTERFACE_CLASSID = "class-id";
   String ICRUD_INTERFACE_GUID    = "guid";

   int FRAME_TYPE_SIZE = 1;
   int SIZE_SIZE       = 4;
   int GUID_SIZE       = 4 + 4;
   int CLASS_ID_SIZE   = 1 + 1 + 1 + 1;
   int HEADER_SIZE     = SIGNATURE.length + FRAME_TYPE_SIZE + SIZE_SIZE + GUID_SIZE + CLASS_ID_SIZE;
   int PAYLOAD_SIZE    =  ( 64*1024 ) - HEADER_SIZE;

   enum FrameType {

      NO_OP,
      DATA_CREATE_OR_UPDATE,
      DATA_DELETE,
      OPERATION
   }
}
