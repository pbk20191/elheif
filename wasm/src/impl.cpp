#include "impl.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <emscripten/emscripten.h>

EMSCRIPTEN_DECLARE_VAL_TYPE(HEIFError)
EMSCRIPTEN_DECLARE_VAL_TYPE(ImageDataType)
EMSCRIPTEN_DECLARE_VAL_TYPE(HEIFDecodeResult)
EMSCRIPTEN_DECLARE_VAL_TYPE(HEIFEncodeResult)
EMSCRIPTEN_DECLARE_VAL_TYPE(PixelFrames)
EMSCRIPTEN_DECLARE_VAL_TYPE(HEIFEncodeResult2)

emscripten::val vector_to_uint8array(const std::vector<uint8_t> &buffer)
{
  using namespace emscripten;

  val uint8array = val::global("Uint8Array").new_(buffer.size());
  val view = val(typed_memory_view(buffer.size(), buffer.data()));
  uint8array.call<void>("set", view);

  return uint8array;
}

HEIFDecodeResult jsDecodeImage(const std::string &buffer)
{
  using namespace emscripten;

  auto res = Elheif::decode(
      reinterpret_cast<const std::uint8_t *>(buffer.data()), buffer.size());

  auto retItems = emscripten::val::array();
  const val ImageData = emscripten::val::global("ImageData");
  const val Error = emscripten::val::global("Error");
  for (const auto &item : res.data)
  {
    auto imageData = ImageData.new_(item.width, item.height);
    auto view = typed_memory_view(item.data.size(), item.data.data());
    auto backing = imageData["data"];
    imageData["data"].call<void>("set", view);
    retItems.call<void>("push", imageData);
  }
  auto ret = emscripten::val::object();
  if (res.error.code == heif_error_Ok) {
    ret.set("data", retItems);
  } else {
    auto error_option = emscripten::val::object();
    error_option.set("cause", res.error);
    const auto jsError = Error.new_(res.err, error_option);
    ret.set("error", jsError);
    // jsError.throw_();
  }

  return HEIFDecodeResult(std::move(ret));
}

static heif_error write_impl(heif_context *ctx, const void *data, size_t size,
                             void *userdata) {
  
  // auto *valuePtr = reinterpret_cast<emscripten::val *>(userdata);
  // assume ArrayBuffer or SharedArrayBuffer
  auto& jsValue = *reinterpret_cast<emscripten::val *>(userdata);
  unsigned byteLength = 0;
  if (jsValue.isUndefined()) {
    jsValue = emscripten::val::global("ArrayBuffer").new_(size);
  } else {
    byteLength = jsValue["byteLength"].as<unsigned>();
    jsValue = jsValue.call<emscripten::val>("transfer",byteLength + size);
  }
  auto view = emscripten::typed_memory_view(size, reinterpret_cast<const std::uint8_t*>(data));
  // std::cout << "call" << std::endl;
  auto slice = emscripten::val::global("Uint8Array").new_(jsValue, byteLength, size);
  slice.call<void>("set", view);
  return heif_error_success;
}


// emscripten::val jsEncodeImages(const emscripten::val &imageDatas) {
//   imageDatas.begin();
//   for (const auto& item : imageDatas) {

//   }
// }

HEIFEncodeResult jsEncodeImage(const ImageDataType &imageData)
{
  const size_t width = imageData["width"].as<size_t>();
  const size_t height = imageData["height"].as<size_t>();
  const auto pixelBuffer = imageData["data"].as<std::string>();
  auto res =
      Elheif::encode(reinterpret_cast<const std::uint8_t *>(pixelBuffer.data()),
                     pixelBuffer.size(), width, height);
  const auto Error = emscripten::val::global("Error");
  auto ret = emscripten::val::object();
  

  if (res.error.code == heif_error_Ok) {
    ret.set("data", vector_to_uint8array(res.data));
  } else {
    auto error_option = emscripten::val::object();
    error_option.set("cause", res.error);
    const auto jsError = Error.new_(res.err, error_option);
    // jsError.throw_();
    ret.set("error", jsError);
  }
  return HEIFEncodeResult(std::move(ret));
}

HEIFEncodeResult2 jsEncodeImages(const PixelFrames &frames)
{
  auto writer = heif_writer {
    .writer_api_version = 1,
    .write = write_impl,
  };
  auto param = emscripten::val::object();
  emscripten::val arrayBuf = emscripten::val::undefined();
  const unsigned length = frames["length"].as<unsigned>();
  auto res = Elheif::encode2(length, [frames](auto idx){
    emscripten::val const& imageData = frames[idx];
  
    const size_t width = imageData["width"].as<size_t>();
    const size_t height = imageData["height"].as<size_t>();
    auto pixelBuffer = imageData["data"].as<std::string>();
    return Elheif::PixelInput{
        .width = width,
        .height = height,
        .data = Elheif::array_view<uint8_t>(
            reinterpret_cast<const uint8_t *>(pixelBuffer.data()),
            pixelBuffer.size())};
  }, writer, &arrayBuf);
  const auto Error = emscripten::val::global("Error");
  auto ret = emscripten::val::object();

  if (res.error.code == heif_error_Ok) {
    ret.set("data", arrayBuf);
  } else {
    auto error_option = emscripten::val::object();
    error_option.set("cause", res.error);
    const auto jsError = Error.new_(res.err, error_option);
    // jsError.throw_();
    ret.set("error", jsError);
  }
  return HEIFEncodeResult2(std::move(ret));
}

EMSCRIPTEN_BINDINGS()
{
  function("jsDecodeImage", &jsDecodeImage, emscripten::return_value_policy::take_ownership());
  function("jsEncodeImage", &jsEncodeImage, emscripten::return_value_policy::take_ownership());
  function("jsEncodeImages", &jsEncodeImages, emscripten::return_value_policy::take_ownership());
  emscripten::register_type<ImageDataType>("ImageData");
  emscripten::register_type<HEIFDecodeResult>("{ data?: ImageData[], error?: Error & { cause: heif_error } }");
  emscripten::register_type<HEIFError>(" Error & { cause: heif_error } ");
  emscripten::register_type<HEIFEncodeResult>("{ error?:Error & { cause: heif_error }, data?: Uint8Array }");
  emscripten::register_type<HEIFEncodeResult2>("{ error?:Error & { cause: heif_error }, data?: ArrayBuffer|SharedArrayBuffer }");
  emscripten::register_type<PixelFrames>("ImageData[]");
    emscripten::enum_<heif_error_code>("heif_error_code")
    .value("heif_error_Ok", heif_error_Ok)
    .value("heif_error_Input_does_not_exist", heif_error_Input_does_not_exist)
    .value("heif_error_Invalid_input", heif_error_Invalid_input)
    .value("heif_error_Plugin_loading_error", heif_error_Plugin_loading_error)
    .value("heif_error_Unsupported_filetype", heif_error_Unsupported_filetype)
    .value("heif_error_Unsupported_feature", heif_error_Unsupported_feature)
    .value("heif_error_Usage_error", heif_error_Usage_error)
    .value("heif_error_Memory_allocation_error", heif_error_Memory_allocation_error)
    .value("heif_error_Decoder_plugin_error", heif_error_Decoder_plugin_error)
    .value("heif_error_Encoder_plugin_error", heif_error_Encoder_plugin_error)
    .value("heif_error_Encoding_error", heif_error_Encoding_error)
    .value("heif_error_End_of_sequence", heif_error_End_of_sequence)
    .value("heif_error_Color_profile_does_not_exist", heif_error_Color_profile_does_not_exist)
    .value("heif_error_Canceled", heif_error_Canceled);

    emscripten::enum_<heif_suberror_code>("heif_suberror_code")
    .value("heif_suberror_Unspecified", heif_suberror_Unspecified)
    .value("heif_suberror_Cannot_write_output_data", heif_suberror_Cannot_write_output_data)
    .value("heif_suberror_Compression_initialisation_error", heif_suberror_Compression_initialisation_error)
    .value("heif_suberror_Decompression_invalid_data", heif_suberror_Decompression_invalid_data)
    .value("heif_suberror_Encoder_initialization", heif_suberror_Encoder_initialization)
    .value("heif_suberror_Encoder_encoding", heif_suberror_Encoder_encoding)
    .value("heif_suberror_Encoder_cleanup", heif_suberror_Encoder_cleanup)
    .value("heif_suberror_Too_many_regions", heif_suberror_Too_many_regions)
    .value("heif_suberror_End_of_data", heif_suberror_End_of_data)
    .value("heif_suberror_Invalid_box_size", heif_suberror_Invalid_box_size)
    .value("heif_suberror_No_ftyp_box", heif_suberror_No_ftyp_box)
    .value("heif_suberror_No_idat_box", heif_suberror_No_idat_box)
    .value("heif_suberror_No_meta_box", heif_suberror_No_meta_box)
    .value("heif_suberror_No_moov_box", heif_suberror_No_moov_box)
    .value("heif_suberror_No_hdlr_box", heif_suberror_No_hdlr_box)
    .value("heif_suberror_No_hvcC_box", heif_suberror_No_hvcC_box)
    .value("heif_suberror_No_vvcC_box", heif_suberror_No_vvcC_box)
    .value("heif_suberror_No_pitm_box", heif_suberror_No_pitm_box)
    .value("heif_suberror_No_ipco_box", heif_suberror_No_ipco_box)
    .value("heif_suberror_No_ipma_box", heif_suberror_No_ipma_box)
    .value("heif_suberror_No_iloc_box", heif_suberror_No_iloc_box)
    .value("heif_suberror_No_iinf_box", heif_suberror_No_iinf_box)
    .value("heif_suberror_No_iprp_box", heif_suberror_No_iprp_box)
    .value("heif_suberror_No_iref_box", heif_suberror_No_iref_box)
    .value("heif_suberror_No_pict_handler", heif_suberror_No_pict_handler)
    .value("heif_suberror_Ipma_box_references_nonexisting_property", heif_suberror_Ipma_box_references_nonexisting_property)
    .value("heif_suberror_No_properties_assigned_to_item", heif_suberror_No_properties_assigned_to_item)
    .value("heif_suberror_No_item_data", heif_suberror_No_item_data)
    .value("heif_suberror_Invalid_grid_data", heif_suberror_Invalid_grid_data)
    .value("heif_suberror_Missing_grid_images", heif_suberror_Missing_grid_images)
    .value("heif_suberror_No_av1C_box", heif_suberror_No_av1C_box)
    .value("heif_suberror_No_avcC_box", heif_suberror_No_avcC_box)
    .value("heif_suberror_Invalid_mini_box", heif_suberror_Invalid_mini_box)
    .value("heif_suberror_Invalid_clean_aperture", heif_suberror_Invalid_clean_aperture)
    .value("heif_suberror_Invalid_overlay_data", heif_suberror_Invalid_overlay_data)
    .value("heif_suberror_Overlay_image_outside_of_canvas", heif_suberror_Overlay_image_outside_of_canvas)
    .value("heif_suberror_Plugin_is_not_loaded", heif_suberror_Plugin_is_not_loaded)
    .value("heif_suberror_Plugin_loading_error", heif_suberror_Plugin_loading_error)
    .value("heif_suberror_Auxiliary_image_type_unspecified", heif_suberror_Auxiliary_image_type_unspecified)
    .value("heif_suberror_Cannot_read_plugin_directory", heif_suberror_Cannot_read_plugin_directory)
    .value("heif_suberror_No_matching_decoder_installed", heif_suberror_No_matching_decoder_installed)
    .value("heif_suberror_No_or_invalid_primary_item", heif_suberror_No_or_invalid_primary_item)
    .value("heif_suberror_No_infe_box", heif_suberror_No_infe_box)
    .value("heif_suberror_Security_limit_exceeded", heif_suberror_Security_limit_exceeded)
    .value("heif_suberror_Unknown_color_profile_type", heif_suberror_Unknown_color_profile_type)
    .value("heif_suberror_Wrong_tile_image_chroma_format", heif_suberror_Wrong_tile_image_chroma_format)
    .value("heif_suberror_Invalid_fractional_number", heif_suberror_Invalid_fractional_number)
    .value("heif_suberror_Invalid_image_size", heif_suberror_Invalid_image_size)
    .value("heif_suberror_Nonexisting_item_referenced", heif_suberror_Nonexisting_item_referenced)
    .value("heif_suberror_Null_pointer_argument", heif_suberror_Null_pointer_argument)
    .value("heif_suberror_Nonexisting_image_channel_referenced", heif_suberror_Nonexisting_image_channel_referenced)
    .value("heif_suberror_Unsupported_plugin_version", heif_suberror_Unsupported_plugin_version)
    .value("heif_suberror_Unsupported_writer_version", heif_suberror_Unsupported_writer_version)
    .value("heif_suberror_Unsupported_parameter", heif_suberror_Unsupported_parameter)
    .value("heif_suberror_Invalid_parameter_value", heif_suberror_Invalid_parameter_value)
    .value("heif_suberror_Invalid_property", heif_suberror_Invalid_property)
    .value("heif_suberror_Item_reference_cycle", heif_suberror_Item_reference_cycle)
    .value("heif_suberror_Invalid_pixi_box", heif_suberror_Invalid_pixi_box)
    .value("heif_suberror_Invalid_region_data", heif_suberror_Invalid_region_data)
    .value("heif_suberror_Unsupported_codec", heif_suberror_Unsupported_codec)
    .value("heif_suberror_Unsupported_image_type", heif_suberror_Unsupported_image_type)
    .value("heif_suberror_Unsupported_data_version", heif_suberror_Unsupported_data_version)
    .value("heif_suberror_Unsupported_generic_compression_method", heif_suberror_Unsupported_generic_compression_method)
    .value("heif_suberror_Unsupported_essential_property", heif_suberror_Unsupported_essential_property)
    .value("heif_suberror_Unsupported_color_conversion", heif_suberror_Unsupported_color_conversion)
    .value("heif_suberror_Unsupported_item_construction_method", heif_suberror_Unsupported_item_construction_method)
    .value("heif_suberror_Unsupported_header_compression_method", heif_suberror_Unsupported_header_compression_method)
    .value("heif_suberror_Unsupported_bit_depth", heif_suberror_Unsupported_bit_depth)
    .value("heif_suberror_Wrong_tile_image_pixel_depth", heif_suberror_Wrong_tile_image_pixel_depth)
    .value("heif_suberror_Unknown_NCLX_color_primaries", heif_suberror_Unknown_NCLX_color_primaries)
    .value("heif_suberror_Unknown_NCLX_transfer_characteristics", heif_suberror_Unknown_NCLX_transfer_characteristics)
    .value("heif_suberror_Unknown_NCLX_matrix_coefficients", heif_suberror_Unknown_NCLX_matrix_coefficients)
    .value("heif_suberror_No_ispe_property", heif_suberror_No_ispe_property)
    .value("heif_suberror_Camera_intrinsic_matrix_undefined", heif_suberror_Camera_intrinsic_matrix_undefined)
    .value("heif_suberror_Camera_extrinsic_matrix_undefined", heif_suberror_Camera_extrinsic_matrix_undefined)
    .value("heif_suberror_Invalid_J2K_codestream", heif_suberror_Invalid_J2K_codestream)
    .value("heif_suberror_No_icbr_box", heif_suberror_No_icbr_box);

    emscripten::value_object<heif_error>("heif_error")
    .field("code", &heif_error::code)
    .field("subcode", &heif_error::subcode)
    .field("message", emscripten::optional_override([](const struct heif_error& err) {
        return std::string(err.message);
    }), emscripten::optional_override([](struct heif_error& err, const std::string& value) {
        err.message = value.c_str();
    }));

}