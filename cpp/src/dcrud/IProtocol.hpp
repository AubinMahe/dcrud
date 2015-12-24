#pragma once

namespace dcrud {

#  define DCRUD_SIGNATURE_SIZE  5U
   extern const byte DCRUD_SIGNATURE[DCRUD_SIGNATURE_SIZE];

#  define FRAME_TYPE_SIZE 1U
#  define SIZE_SIZE       4U
#  define GUID_SIZE       ( 4U + 4U )
#  define CLASS_ID_SIZE   ( 1U + 1U + 1U + 1U )
#  define HEADER_SIZE     ( DCRUD_SIGNATURE_SIZE + FRAME_TYPE_SIZE + SIZE_SIZE + GUID_SIZE + CLASS_ID_SIZE )
#  define PAYLOAD_SIZE    ( 64U*1024U - HEADER_SIZE )

   enum FrameType {
      FRAMETYPE_NO_OP,
      FRAMETYPE_DATA_CREATE_OR_UPDATE,
      FRAMETYPE_DATA_DELETE,
      FRAMETYPE_OPERATION
   };
#  define ICRUD_INTERFACE_NAME    "dcrud.ICRUD"
#  define ICRUD_INTERFACE_CREATE  "create"
#  define ICRUD_INTERFACE_UPDATE  "update"
#  define ICRUD_INTERFACE_DELETE  "delete"
#  define ICRUD_INTERFACE_CLASSID "class-id"
#  define ICRUD_INTERFACE_GUID    "guid"

#  define INTERFACE_NAME_MAX_LENGTH 1000
#  define OPERATION_NAME_MAX_LENGTH 1000
#  define ARG_NAME_MAX_LENGTH       1000
}