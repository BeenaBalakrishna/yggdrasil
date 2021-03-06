#ifndef ASCII_TABLE_METASCHEMA_TYPE_H_
#define ASCII_TABLE_METASCHEMA_TYPE_H_

#include "../tools.h"
#include "MetaschemaType.h"
#include "../dataio/AsciiTable.h"

#ifndef __cplusplus /* If this is a C compiler, use C++ linkage */
//extern "C++" {
#endif

#include "rapidjson/document.h"
#include "rapidjson/writer.h"


/*!
  @brief Class for ascii table serializaiton/deserialization.

  The AsciiTableMetaschemaType provides basic functionality for encoding/decoding
  table datatypes from/to JSON style strings.
 */
class AsciiTableMetaschemaType : public MetaschemaType {
public:
  /*!
    @brief Constructor for AsciiTableMetaschemaType.
    @param[in] format_str char * Format string describing table structure.
    @param[in] as_array int (optional) If 1, the instance will act to
    serialize/deserialize table columns. If 0, the instnace will act to
    serialize/deserialize table rows. Defaults to 0.
   */
  AsciiTableMetaschemaType(const char *format_str, const int as_array=0) :
    MetaschemaType("ascii_table"), as_array_(as_array), table_(NULL) {
    table_ = (asciiTable_t*)malloc(sizeof(asciiTable_t));
    if (table_ == NULL)
      ygglog_throw_error("AsciiTableMetaschemaType: Failed to allocate table.");
    table_[0] = asciiTable("seri", "0", format_str, NULL, NULL, NULL);
  }
  /*!
    @brief Destructor for AsciiTableMetaschemaType.
    Free the table structure created during constructor.
   */
  ~AsciiTableMetaschemaType() {
    if (table_ != NULL) {
      at_cleanup(table_);
      free(table_);
    }
  }
  /*!
    @brief Create a copy of the type.
    @returns pointer to new AsciiTableMetaschemaType instance with the same data.
   */
  AsciiTableMetaschemaType* copy() { return (new AsciiTableMetaschemaType(format_str(),
									  as_array())); }
  /*!
    @brief Print information about the type to stdout.
  */
  void display() {
    MetaschemaType::display();
    printf("%-15s = %s\n", "format_str", format_str());
    printf("%-15s = %d\n", "as_array", as_array_);
  }
  /*!
    @brief Get format string describing table.
    @returns char * Format string.
   */
  const char* format_str() { return table_->format_str; }
  /*!
    @brief Get table struct.
    @returns asciiTable_t* Table struct.
   */
  asciiTable_t* table() { return table_; }
  /*!
    @brief Get as_array.
    @returns bool 1 if elements in table are all arrays, 0 otherwise.
   */
  const int as_array() { return as_array_; }
  /*!
    @brief Get the number of arguments expected to be filled/used by the type.
    @returns size_t Number of arguments.
   */
  virtual size_t nargs_exp() {
    size_t nargs = (size_t)(table_->ncols);
    if (as_array_) {
      nargs++; // For the number of rows
    }
    return nargs;
  }
  
  // Encoding
  /*!
    @brief Encode arguments describine an instance of this type into a JSON string.
    @param[in] writer rapidjson::Writer<rapidjson::StringBuffer> rapidjson writer.
    @param[in,out] nargs size_t * Pointer to the number of arguments contained in
    ap. On return it will be set to the number of arguments used.
    @param[in] ap va_list_t Variable number of arguments that should be encoded
    as a JSON string.
    @returns bool true if the encoding was successful, false otherwise.
   */
  bool encode_data(rapidjson::Writer<rapidjson::StringBuffer> *writer,
		   size_t *nargs, va_list_t &ap) {
    // Prevent C4100 warning on windows by referencing param
#ifdef _WIN32
    writer;
    nargs;
    ap;
#endif
    ygglog_error("AsciiTableMetaschemaType::encode_data: AsciiTable type cannot be JSON encoded.");
    return false;
  }
  /*!
    @brief Serialize an instance including it's type and data.
    @param[out] buf char ** Buffer where serialized data should be written.
    @param[in,out] buf_siz size_t* Size of buf. If buf is reallocated, the
    new size of the buffer will be assigned to this address.
    @param[in] allow_realloc int If 1, buf will be reallocated if it is not
    large enough to contain the serialized data. If 0, an error will be raised
    if it is not large enough.
    @param[in,out] nargs size_t Number of arguments contained in ap. On output
    the number of arguments used will be assigned to this address.
    @param[in] ap va_list_t Variable number of arguments that will be serialized.
    @returns int Size of the serialized data in buf.
   */
  int serialize(char **buf, size_t *buf_siz,
		const int allow_realloc, size_t *nargs, va_list_t &ap) {
    if (nargs_exp() != *nargs) {
      ygglog_throw_error("AsciiTableMetaschemaType::serialize: %d arguments expected, but %d provided.",
			 nargs_exp(), *nargs);
    }
    *nargs = *nargs - nargs_exp();
    // Assumes null termination
    int ret;
    if (as_array_) {
      ret = at_varray_to_bytes(*table_, *buf, *buf_siz, ap.va);
    } else {
      ret = at_vrow_to_bytes(*table_, *buf, *buf_siz, ap.va);
    }
    if (*nargs != 0) {
      ygglog_error("AsciiTableMetaschemaType::serialize: %d arguments were not used.", *nargs);
      return -1;
    }
    return ret;
  }
  
  // Decoding
  /*!
    @brief Decode variables from a JSON string.
    @param[in] data rapidjson::Value Reference to entry in JSON string.
    @param[in] allow_realloc int If 1, the passed variables will be reallocated
    to contain the deserialized data.
    @param[in,out] nargs size_t Number of arguments contained in ap. On return,
    the number of arguments assigned from the deserialized data will be assigned
    to this address.
    @param[out] ap va_list_t Reference to variable argument list containing
    address where deserialized data should be assigned.
    @returns bool true if the data was successfully decoded, false otherwise.
   */
  bool decode_data(rapidjson::Value &data, const int allow_realloc,
		   size_t *nargs, va_list_t &ap) {
    // Prevent C4100 warning on windows by referencing param
#ifdef _WIN32
    data;
    allow_realloc;
    nargs;
    ap;
#endif
    ygglog_error("AsciiTableMetaschemaType::decode_data: AsciiTable type cannot be JSON decoded.");
    return false;
  }
  /*!
    @brief Deserialize variables from a JSON string.
    @param[in] buf char* Buffer containing serialized data.
    @param[in] buf_siz size_t Size of the serialized data.
    @param[in] allow_realloc int If 1, the provided variables will be realloced
    as necessary to house the deserialized data.
    @param[in,out] nargs size_t* Number of arguments contained in ap. On
    return, the number of arguments assigned will be assigned to this address.
    @param[out] ap va_list_t Arguments that should be assigned based on the
    deserialized data.
    @returns int -1 if there is an error, otherwise the number of arguments
    remaining in ap.
   */
  int deserialize(const char *buf, const size_t buf_siz,
		  const int allow_realloc, size_t *nargs, va_list_t &ap) {
    if (nargs_exp() != *nargs) {
      ygglog_throw_error("AsciiTableMetaschemaType::deserialize: %d arguments expected, but %d provided.",
			 nargs_exp(), *nargs);
    }
    const size_t nargs_orig = *nargs;
    *nargs = *nargs - nargs_exp();
    int ret;
    if (as_array_) {
      ret = at_vbytes_to_array(*table_, buf, buf_siz, ap.va);
    } else {
      if (allow_realloc) {
	ygglog_error("AsciiTableMetaschemaType::deserialize: allow_realloc not supported for rows.");
	return -1;
      }
      ret = at_vbytes_to_row(*table_, buf, ap.va);
    }
    if (ret < 0) {
      ygglog_error("AsciiTableMetaschemaType::deserialize: Error using table.");
      return -1;
    } else if (ret != nargs_exp()) {
      ygglog_error("AsciiTableMetaschemaType::deserialize: Table used %d arguments, but was expected to used %d.",
		   ret, nargs_exp());
      return -1;
    }
    if (*nargs != 0) {
      ygglog_error("AsciiTableMetaschemaType::deserialize: %d arguments were not used.", *nargs);
      return -1;
    }
    return (int)(nargs_orig - *nargs);
  }

private:
  const int as_array_;
  asciiTable_t *table_;

};

#ifndef __cplusplus /* If this is a C compiler, end C++ linkage */
//}
#endif

#endif /*ASCII_TABLE_METASCHEMA_TYPE_H_*/
// Local Variables:
// mode: c++
// End:
