#include "include/native_pdf_renderer/native_pdf_renderer_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <comdef.h>
#include "src/utility/global.cpp"
#include "src/utility/func.hpp"
#include "src/resources/repository.cpp"

#include "src/resources/document_repository.cpp"
#include "src/resources/page_repository.cpp"

using namespace global;

//#include "mupdf/fitz.h"
//#include "mupdf/pdf.h"
#pragma comment(lib,"lib/libmupdf.lib")

namespace {

class NativePdfRendererPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  NativePdfRendererPlugin();

  virtual ~NativePdfRendererPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  DocumentRepository documents = DocumentRepository();
  PageRepository pages = PageRepository();

  void HandleMethodCall(
      const flutter::MethodCall<> &method_call,
      std::unique_ptr<flutter::MethodResult<>> result);
  fz_document* openAssetDocument(char *filename);
};

// static
void NativePdfRendererPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  fz_register_document_handlers(ctx);

  auto channel =
      std::make_unique<flutter::MethodChannel<>>(
          registrar->messenger(), "io.scer.pdf.renderer",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<NativePdfRendererPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

NativePdfRendererPlugin::NativePdfRendererPlugin() {}

NativePdfRendererPlugin::~NativePdfRendererPlugin() {}

fz_buffer* render(char *filename, int pagenumber, int zoom, int rotation)
{
    // Create a context to hold the exception stack and various caches.
    // fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    // fz_register_document_handlers(ctx);
    // Open the PDF, XPS or CBZ document.
    fz_document *doc = fz_open_document(ctx, filename);
    // Retrieve the number of pages (not used in this example).
    int pagecount = fz_count_pages(ctx, doc);
    // Load the page we want. Page numbering starts from zero.
    fz_page *page = fz_load_page(ctx, doc, pagenumber - 1);
    // Calculate a transform to use when rendering. This transform
    // contains the scale and rotation. Convert zoom percentage to a
    // scaling factor. Without scaling the resolution is 72 dpi.
    fz_matrix transform = fz_rotate(rotation);
    fz_pre_scale(transform, zoom / 100.0f, zoom / 100.0f);
    // Take the page bounds and transform them by the same matrix that
    // we will use to render the page.
    fz_rect bounds = fz_bound_page(ctx, page);
    fz_transform_rect(bounds, transform);
    // Create a blank pixmap to hold the result of rendering. The
    // pixmap bounds used here are the same as the transformed page
    // bounds, so it will contain the entire page. The page coordinate
    // space has the origin at the top left corner and the x axis
    // extends to the right and the y axis extends down.
    fz_irect bbox = fz_round_rect(bounds);
    fz_separations *seps = fz_page_separations(ctx, page);
    fz_pixmap *pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), bbox, seps, 0);
    fz_clear_pixmap_with_value(ctx, pix, 0xff);
    // A page consists of a series of objects (text, line art, images,
    // gradients). These objects are passed to a device when the
    // interpreter runs the page. There are several devices, used for
    // different purposes:
    //
    //    draw device -- renders objects to a target pixmap.
    //
    //    text device -- extracts the text in reading order with styling
    //    information. This text can be used to provide text search.
    //
    //    list device -- records the graphic objects in a list that can
    //    be played back through another device. This is useful if you
    //    need to run the same page through multiple devices, without
    //    the overhead of parsing the page each time.
    // Create a draw device with the pixmap as its target.
    // Run the page with the transform.
    fz_device *dev = fz_new_draw_device(ctx, transform, pix);
    fz_run_page(ctx, page, dev, transform, NULL);
    fz_drop_device(ctx, dev);
    // Save the pixmap to a file.
    //fz_save_pixmap_as_png(ctx, pix, "out.png");
    fz_buffer *buf = fz_new_buffer_from_pixmap_as_png(ctx, pix, fz_default_color_params);
    // Clean up.
    fz_drop_pixmap(ctx, pix);
    fz_drop_page(ctx, page);
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);

    return buf;
}

fz_document* NativePdfRendererPlugin::openAssetDocument(char *filename) {
  fz_document *doc = fz_open_document(ctx, filename);
  return doc;
}

void NativePdfRendererPlugin::HandleMethodCall(
    const flutter::MethodCall<> &method_call,
    std::unique_ptr<flutter::MethodResult<>> result) {
  // Replace "getPlatformVersion" check with your plugin's method.
  // See:
  // https://github.com/flutter/engine/tree/master/shell/platform/common/cpp/client_wrapper/include/flutter
  // and
  // https://github.com/flutter/engine/tree/master/shell/platform/glfw/client_wrapper/include/flutter
  // for the relevant Flutter APIs.
  if (method_call.method_name().compare("open.document.asset") == 0) {
    const std::string *path = std::get_if<std::string>(method_call.arguments());

    //获取程序路径
    std::wstring wstr; 
    unsigned long size=GetCurrentDirectory(0,NULL); 
    wchar_t *path1=new wchar_t[size]; 
    if(GetCurrentDirectory(size, path1)!=0) 
    { 
        wstr=path1; 
    }

    //拼接assets路径
    char filename[MAX_PATH];
    sprintf(filename, "%ws%c%s", wstr.c_str(), '/', path->c_str());

    fz_document *doc = openAssetDocument(filename);

    result->Success(documents.registerEx(doc).infoMap());
  } else if(method_call.method_name().compare("open.document.data") == 0) {
    const auto *data = std::get_if<std::vector<uint8_t>>(method_call.arguments());
    const uint8_t *u8data = &(*data->begin());
    const unsigned char *cdata = (const unsigned char *)u8data;

    size_t len = data->size();

    unsigned char *cdataArray = new unsigned char[len];

    //参数数据会被回收，需要拷贝，不然后续读不到数据
    memcpy(cdataArray,cdata,len);

    fz_stream *stream = fz_open_memory(ctx, cdataArray, len);
    fz_document *doc = fz_open_document_with_stream(ctx, "application/pdf", stream);
    
    result->Success(documents.registerEx(doc, stream, cdataArray).infoMap());
  } else if(method_call.method_name().compare("open.page") == 0) {
    const flutter::EncodableMap *args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    const auto *documentId = std::get_if<string>(ValueOrNull(*args, "documentId"));
    const auto *pageNumber = std::get_if<int>(ValueOrNull(*args, "page"));
    std::cout << *pageNumber << std::endl;
    std::cout << *documentId << std::endl;
    Document doc = documents.get(*documentId);
    std::cout << "open.page Document" << std::endl;
    fz_page *renderer = doc.openPage(*pageNumber);
    std::cout << "open.page fz_page" << std::endl;
    Page page = pages.registerEx(*documentId, renderer);
    std::cout << "open.page Page" << std::endl;
    result->Success(page.infoMap());
  } else if(method_call.method_name().compare("render") == 0) {
    const flutter::EncodableMap *args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    const auto *pageId = std::get_if<string>(ValueOrNull(*args, "pageId"));
    const auto *width = std::get_if<int>(ValueOrNull(*args, "width"));
    const auto *height = std::get_if<int>(ValueOrNull(*args, "height"));
    const auto *crop = std::get_if<bool>(ValueOrNull(*args, "crop"));
    const auto *format = std::get_if<int>(ValueOrNull(*args, "format"));
    const auto *backgroundColor = std::get_if<string>(ValueOrNull(*args, "backgroundColor"));

    Page page = pages.get(*pageId);
    DataResult *data = page.render(200, 200);

    flutter::EncodableMap results;

    results.insert(std::pair<string, int>("width", data->width));
    results.insert(std::pair<string, int>("height", data->height));
    results.insert(std::pair<string, flutter::EncodableValue>("data", data->data));

    result->Success(results);
  } else if(method_call.method_name().compare("close.page") == 0) {
    const std::string *id = std::get_if<std::string>(method_call.arguments());
    pages.close(*id);
    result->Success(NULL);
  } else if (method_call.method_name().compare("test") == 0) {
    const std::string *path = std::get_if<std::string>(method_call.arguments());

    //获取程序路径
    std::wstring wstr; 
    unsigned long size=GetCurrentDirectory(0,NULL); 
    wchar_t *path1=new wchar_t[size]; 
    if(GetCurrentDirectory(size, path1)!=0) 
    { 
        wstr=path1; 
    }

    //拼接assets路径
    char filename[MAX_PATH];
    sprintf(filename, "%ws%c%s", wstr.c_str(), '/', path->c_str());
    std::cout << filename << std::endl;

    //获取pdf页图片
    fz_buffer *buf = render(filename, 1, 100, 0);
    unsigned char *data = buf->data;

    std::vector<uint8_t> vecData_;
    vecData_.insert(vecData_.end(), data, data + buf->len);
    result->Success(flutter::EncodableValue(vecData_));

    // const auto *path = std::get_if<std::string>(method_call.arguments());

    // std::wstring wstr; 
    // unsigned long size=GetCurrentDirectory(0,NULL); 
    // wchar_t *path1=new wchar_t[size]; 
    // if(GetCurrentDirectory(size, path1)!=0) 
    // { 
    //     wstr=path1; 
    // } 
    // delete [] path;
    // printf((char *)path);

    // std::ostringstream version_stream;
    // version_stream << "Windows ";
    // if (IsWindows10OrGreater()) {
    //   version_stream << "10+";
    // } else if (IsWindows8OrGreater()) {
    //   version_stream << "8";
    // } else if (IsWindows7OrGreater()) {
    //   version_stream << "7";
    // }
    // result->Success(flutter::EncodableValue(version_stream.str()));
  } else {
    result->NotImplemented();
  }
}

}  // namespace

void NativePdfRendererPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  NativePdfRendererPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
