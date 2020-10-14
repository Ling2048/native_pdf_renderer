#include <string>
#include <map>
#include <flutter/standard_method_codec.h>

using std::string;

#include "../utility/global.h"
//#include "../mupdf/fitz.h"
//using namespace global;

enum CompressFormat {
    JPEG = 0,
    PNG = 1,
};

namespace {
    class DataResult
    {
        private:
            /* data */
        public:
            DataResult(/* args */);
            ~DataResult();
            int width;
            int height;
            flutter::EncodableValue data;
    };
    
    DataResult::DataResult(/* args */)
    {
    }
    
    DataResult::~DataResult()
    {
    }

    class Page
    {
        private:
            /* data */
            string id;
            string documentId;
            fz_page *renderer;
            string boxRect;
        public:
            Page();
            Page(string id, string documentId, fz_page *renderer);
            ~Page();
            int number();
            int width();
            int height();
            flutter::EncodableMap infoMap();
            DataResult* render(int width, int height);
    };

    int Page::number() {
        return 1;
    }

    int Page::width() {
        return 200;
    }

    int Page::height() {
        return 200;
    }

    flutter::EncodableMap Page::infoMap() {
        flutter::EncodableMap map;

        map.insert(std::pair<std::string, std::string>("documentId", this->documentId));
        map.insert(std::pair<std::string, std::string>("id", this->id));
        map.insert(std::pair<std::string, int>("pageNumber", number()));
        map.insert(std::pair<std::string, int>("width", width()));
        map.insert(std::pair<std::string, int>("height", height()));

        return map;
    }

    DataResult* Page::render(int width, int height) {
        DataResult *data = new DataResult();

        // Calculate a transform to use when rendering. This transform
        // contains the scale and rotation. Convert zoom percentage to a
        // scaling factor. Without scaling the resolution is 72 dpi.
        fz_matrix transform = fz_rotate(0);
        fz_pre_scale(transform, 1 / 100.0f, 1 / 100.0f);
        // Take the page bounds and transform them by the same matrix that
        // we will use to render the page.
        fz_rect bounds = fz_bound_page(ctx, renderer);
        fz_transform_rect(bounds, transform);
        // Create a blank pixmap to hold the result of rendering. The
        // pixmap bounds used here are the same as the transformed page
        // bounds, so it will contain the entire page. The page coordinate
        // space has the origin at the top left corner and the x axis
        // extends to the right and the y axis extends down.
        fz_irect bbox = fz_round_rect(bounds);
        fz_separations *seps = fz_page_separations(ctx, renderer);
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
        fz_run_page(ctx, renderer, dev, transform, NULL);
        fz_drop_device(ctx, dev);
        // Save the pixmap to a file.
        //fz_save_pixmap_as_png(ctx, pix, "out.png");
        fz_buffer *buf = fz_new_buffer_from_pixmap_as_png(ctx, pix, fz_default_color_params);

        unsigned char *dataBuf = buf->data;

        std::vector<uint8_t> vecData_;
        vecData_.insert(vecData_.end(), dataBuf, dataBuf + buf->len);

        data->data = flutter::EncodableValue(vecData_);
        data->width = this->width();
        data->height = this->height();

        return data;
    }

    Page::Page() {}
    
    Page::Page(string id, string documentId, fz_page *renderer)
    {
        this->id = id;
        this->documentId = documentId;
        this->renderer = renderer;
    }
    
    Page::~Page()
    {
    }
}