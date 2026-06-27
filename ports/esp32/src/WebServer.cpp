#include "WebServer.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <esp_heap_caps.h>

#include <multigauge/App.h>
#include <multigauge/io/Log.h>
#include <multigauge/utils/Json.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace mgweb {

namespace {

constexpr const char* TAG = "WebUpload";

AsyncWebServer server(80);
mg::ContextId g_contextId = 0;

std::string g_uploadBuffer;
std::string g_uploadFilename;
bool g_uploadReady = false;
bool g_uploadOk = false;
bool g_uploadPending = false;
bool g_uploadProcessing = false;
std::string g_uploadError;

String escapeHtml(const std::string& input) {
    String out;
    for (char c : input) {
        switch (c) {
            case '&': out += F("&amp;"); break;
            case '<': out += F("&lt;"); break;
            case '>': out += F("&gt;"); break;
            case '"': out += F("&quot;"); break;
            case '\'': out += F("&#39;"); break;
            default: out += c; break;
        }
    }
    return out;
}

std::string getStringField(const rapidjson::Value& object, const char* key) {
    std::string out;
    if (mg::json::getStringMember(object, key, out)) {
        return out;
    }
    return {};
}

bool getPostPackageId(AsyncWebServerRequest* request, std::string& outPackageId) {
    if (!request->hasParam("packageId", true)) {
        return false;
    }

    outPackageId = request->getParam("packageId", true)->value().c_str();
    return !outPackageId.empty();
}

void logHeapStats(const char* stage) {
    LOG_INFO(TAG,
             "%s: freeHeap=%u minFreeHeap=%u largest8bit=%u",
             stage,
             static_cast<unsigned>(ESP.getFreeHeap()),
             static_cast<unsigned>(ESP.getMinFreeHeap()),
             static_cast<unsigned>(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)));
}

String makeIndexPage() {
    String html;
    html.reserve(16384);

    const size_t totalBytes = LittleFS.totalBytes();
    const size_t usedBytes = LittleFS.usedBytes();
    const size_t freeBytes = totalBytes > usedBytes ? totalBytes - usedBytes : 0;
    const unsigned storagePercent = totalBytes > 0 ? static_cast<unsigned>((usedBytes * 100u) / totalBytes) : 0u;

    html += F("<!doctype html><html><head><meta charset='utf-8'>");
    html += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
    html += F("<title>Multigauge</title>");
    html += F("<style>");
    html += F("body{margin:0;font-family:Arial,sans-serif;background:#f3f5f7;color:#1f2933;}");
    html += F(".wrap{max-width:960px;margin:0 auto;padding:24px;}");
    html += F("h1{margin:0 0 8px;font-size:2rem;}");
    html += F(".grid{display:grid;grid-template-columns:1fr;gap:16px;}");
    html += F(".card{background:#fff;border:1px solid #d9e2ec;border-radius:14px;padding:16px;box-shadow:0 2px 8px rgba(16,24,40,.05);}");
    html += F(".card-head{display:flex;justify-content:space-between;gap:12px;align-items:flex-start;}");
    html += F(".title{margin:0;font-size:1.25rem;}");
    html += F(".badge{display:inline-block;background:#e0f2fe;color:#075985;border-radius:999px;padding:4px 10px;font-size:.8rem;white-space:nowrap;}");
    html += F(".meta{margin:12px 0 0;display:grid;grid-template-columns:auto 1fr;gap:6px 12px;font-size:.95rem;}");
    html += F(".meta dt{color:#627d98;font-weight:600;}");
    html += F(".meta dd{margin:0;}");
    html += F(".description{margin:12px 0 0;white-space:pre-wrap;line-height:1.45;color:#334e68;}");
    html += F(".face-actions{margin-top:14px;display:flex;flex-wrap:wrap;gap:10px;}");
    html += F(".package-actions{margin-top:12px;display:flex;flex-wrap:wrap;gap:10px;align-items:center;}");
    html += F("button,input{font:inherit;}");
    html += F(".face-button{padding:9px 14px;border:1px solid #cbd2d9;border-radius:999px;background:#f8fafc;color:#1f2933;cursor:pointer;}");
    html += F(".face-button:hover{background:#e2e8f0;}");
    html += F(".secondary-button{padding:9px 14px;border:1px solid #cbd2d9;border-radius:10px;background:#fff;color:#1f2933;cursor:pointer;}");
    html += F(".danger-button{padding:9px 14px;border:1px solid #ef4444;border-radius:10px;background:#fff;color:#b91c1c;cursor:pointer;}");
    html += F(".danger-button:hover{background:#fef2f2;}");
    html += F(".empty{padding:16px;border:1px dashed #cbd2d9;border-radius:12px;color:#627d98;background:#fff;}");
    html += F(".section{margin-top:28px;}");
    html += F(".upload{display:flex;flex-wrap:wrap;gap:10px;align-items:center;}");
    html += F(".upload button{padding:9px 14px;border:0;border-radius:10px;background:#0f766e;color:#fff;cursor:pointer;}");
    html += F(".storage{margin:0 0 18px;background:#fff;border:1px solid #d9e2ec;border-radius:14px;padding:14px 16px;box-shadow:0 2px 8px rgba(16,24,40,.05);}");
    html += F(".storage-top{display:flex;justify-content:space-between;gap:12px;align-items:baseline;}");
    html += F(".storage-bar{margin-top:10px;height:12px;background:#edf2f7;border-radius:999px;overflow:hidden;}");
    html += F(".storage-fill{height:100%;background:linear-gradient(90deg,#0f766e,#14b8a6);}");
    html += F(".storage-text{margin-top:8px;color:#52606d;font-size:.92rem;}");
    html += F("</style></head><body><div class='wrap'>");
    html += F("<h1>Multigauge</h1>");

    html += F("<div class='storage'>");
    html += F("<div class='storage-top'><strong>Storage</strong><span>");
    html += String(freeBytes / 1024u);
    html += F(" KB free</span></div>");
    html += F("<div class='storage-bar'><div class='storage-fill' style='width:");
    html += String(storagePercent);
    html += F("%'></div></div>");
    html += F("<div class='storage-text'>");
    html += String(usedBytes / 1024u);
    html += F(" KB used of ");
    html += String(totalBytes / 1024u);
    html += F(" KB total</div></div>");

    std::vector<mg::PackageSummary> packages;
    if (mg::listPackages(packages)) {
        LOG_INFO(TAG, "makeIndexPage: packages=%u", static_cast<unsigned>(packages.size()));
        html += F("<div class='grid'>");
        for (const auto& package : packages) {
            LOG_INFO(TAG, "makeIndexPage: packageId=%s name=%s", package.id.c_str(), package.name.c_str());

            std::vector<mg::FaceSummary> faces;
            if (!mg::listFaces(package.id, faces)) {
                LOG_WARN(TAG, "makeIndexPage: no faces for packageId=%s", package.id.c_str());
                continue;
            }

            std::string description = "No description available.";
            const mg::Result packageDoc = mg::getPackage(package.id);
            if (packageDoc.ok && packageDoc.data.IsObject()) {
                std::string maybeDescription = getStringField(packageDoc.data, "description");
                if (!maybeDescription.empty()) {
                    description = std::move(maybeDescription);
                }
            }

            html += F("<section class='card'>");
            html += F("<div class='card-head'>");
            html += F("<div><h2 class='title'>");
            html += escapeHtml(package.name);
            html += F("</h2></div>");
            html += F("<div class='badge'>");
            html += String(faces.size());
            html += F(" faces</div>");
            html += F("</div>");

            html += F("<dl class='meta'>");
            html += F("<dt>Author</dt><dd>");
            html += escapeHtml(package.author);
            html += F("</dd><dt>Description</dt><dd class='description'>");
            html += escapeHtml(description);
            html += F("</dd></dl>");

            html += F("<form class='face-actions' method='post' action='/show'>");
            for (const auto& face : faces) {
                LOG_DEBUG(TAG, "makeIndexPage: face packageId=%s faceId=%s name=%s", package.id.c_str(), face.id.c_str(), face.name.c_str());
                const std::string selection = package.id + "|" + face.id;
                html += F("<button class='face-button' type='submit' name='selection' value='");
                html += escapeHtml(selection);
                html += F("'>");
                html += escapeHtml(face.name);
                html += F("</button>");
            }
            html += F("</form>");

            html += F("<div class='package-actions'>");
            html += F("<form method='post' action='/export'>");
            html += F("<input type='hidden' name='packageId' value='");
            html += escapeHtml(package.id);
            html += F("'>");
            html += F("<button class='secondary-button' type='submit'>Export package</button>");
            html += F("</form>");
            html += F("<form method='post' action='/remove' onsubmit='return confirm(\"Remove this package?\")'>");
            html += F("<input type='hidden' name='packageId' value='");
            html += escapeHtml(package.id);
            html += F("'>");
            html += F("<button class='danger-button' type='submit'>Remove package</button>");
            html += F("</form>");
            html += F("</div>");
            html += F("</section>");
        }
        html += F("</div>");
    } else {
        LOG_WARN(TAG, "makeIndexPage: package summaries unavailable");
    }

    if (packages.empty()) {
        html += F("<div class='empty'>No gauges installed yet.</div>");
    }

    html += F("<div class='section card'>");
    html += F("<h2 class='title' style='font-size:1.05rem;'>Upload package</h2>");
    html += F("<form class='upload' method='post' action='/upload' enctype='multipart/form-data'>");
    html += F("<label for='package'>Package JSON</label>");
    html += F("<input id='package' type='file' name='package' accept='.json,application/json'>");
    html += F("<button type='submit'>Upload package</button>");
    html += F("</form>");
    html += F("</div>");

    html += F("</div></body></html>");
    return html;
}

bool normalizeUpload(const std::string& json, rapidjson::Document& outDoc, std::string& outError) {
    LOG_INFO(TAG, "normalizeUpload: bytes=%u", static_cast<unsigned>(json.size()));
    rapidjson::Document input = mg::json::parseJson(json);
    if (input.HasParseError()) {
        outError = "Invalid JSON";
        LOG_WARN(TAG, "normalizeUpload: parse failed");
        return false;
    }

    if (!input.IsObject()) {
        outError = "Package JSON must be an object";
        LOG_WARN(TAG, "normalizeUpload: top-level is not an object");
        return false;
    }

    const char* requiredKeys[] = {"name", "author", "description", "faces"};
    for (const char* key : requiredKeys) {
        if (!input.HasMember(key)) {
            outError = std::string("Package JSON is missing required field: ") + key;
            LOG_WARN(TAG, "normalizeUpload: missing field %s", key);
            return false;
        }
    }

    if (input.MemberCount() != 4) {
        outError = "Package JSON contains unsupported fields";
        LOG_WARN(TAG, "normalizeUpload: unsupported top-level fields");
        return false;
    }

    if (!input["name"].IsString() || input["name"].GetStringLength() == 0) {
        outError = "Package name must be a non-empty string";
        LOG_WARN(TAG, "normalizeUpload: invalid name");
        return false;
    }

    if (!input["author"].IsString() || input["author"].GetStringLength() == 0) {
        outError = "Package author must be a non-empty string";
        LOG_WARN(TAG, "normalizeUpload: invalid author");
        return false;
    }

    if (!input["description"].IsString()) {
        outError = "Package description must be a string";
        LOG_WARN(TAG, "normalizeUpload: invalid description");
        return false;
    }

    if (!input["faces"].IsArray() || input["faces"].Empty()) {
        outError = "Package must contain at least one face";
        LOG_WARN(TAG, "normalizeUpload: faces missing or empty");
        return false;
    }

    for (const auto& faceEntry : input["faces"].GetArray()) {
        if (!faceEntry.IsObject()) {
            outError = "Face entries must be objects";
            LOG_WARN(TAG, "normalizeUpload: face entry is not object");
            return false;
        }

        if (!faceEntry.HasMember("name") || !faceEntry.HasMember("face")) {
            outError = "Face entries must contain name and face";
            LOG_WARN(TAG, "normalizeUpload: face entry missing fields");
            return false;
        }

        if (faceEntry.MemberCount() != 2) {
            outError = "Face entries contain unsupported fields";
            LOG_WARN(TAG, "normalizeUpload: face entry has unsupported fields");
            return false;
        }

        if (!faceEntry["name"].IsString() || faceEntry["name"].GetStringLength() == 0) {
            outError = "Face name must be a non-empty string";
            LOG_WARN(TAG, "normalizeUpload: face entry invalid name");
            return false;
        }

        if (!faceEntry["face"].IsObject()) {
            outError = "Face payload must be an object";
            LOG_WARN(TAG, "normalizeUpload: face payload invalid");
            return false;
        }
    }

    outDoc.Swap(input);
    return true;
}

void handleIndex(AsyncWebServerRequest* request) {
    logHeapStats("makeIndexPage before");
    String html = makeIndexPage();
    logHeapStats("makeIndexPage after");
    request->send(200, "text/html", html);
}

void handleShow(AsyncWebServerRequest* request) {
    if (!request->hasParam("selection", true)) {
        request->send(400, "text/plain", "Missing selection");
        return;
    }

    const String selection = request->getParam("selection", true)->value();
    const int separator = selection.indexOf('|');
    if (separator <= 0 || separator >= selection.length() - 1) {
        request->send(400, "text/plain", "Invalid selection");
        return;
    }

    const std::string packageId = selection.substring(0, separator).c_str();
    const std::string faceId = selection.substring(separator + 1).c_str();

    if (!mg::setGaugeScreen(g_contextId, packageId, faceId)) {
        request->send(400, "text/plain", "Failed to show gauge");
        return;
    }

    request->redirect("/");
}

void handleExport(AsyncWebServerRequest* request) {
    std::string packageId;
    if (!getPostPackageId(request, packageId)) {
        request->send(400, "text/plain", "Missing packageId");
        return;
    }

    const mg::Result package = mg::exportPackage(packageId);
    if (!package.ok) {
        request->send(404, "text/plain", package.error.c_str());
        return;
    }

    const std::string json = mg::json::toString(package.data);
    AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json.c_str());

    String filename = "attachment; filename=\"";
    filename += packageId.c_str();
    filename += F(".json\"");
    response->addHeader("Content-Disposition", filename);
    request->send(response);
}

void handleRemove(AsyncWebServerRequest* request) {
    std::string packageId;
    if (!getPostPackageId(request, packageId)) {
        request->send(400, "text/plain", "Missing packageId");
        return;
    }

    const mg::Result result = mg::removePackage(packageId);
    if (!result.ok) {
        request->send(400, "text/plain", result.error.c_str());
        return;
    }

    request->redirect("/");
}

void handleUpload(AsyncWebServerRequest* request) {
    if (!g_uploadReady) {
        for (int i = 0; i < 200 && !g_uploadReady; ++i) {
            delay(10);
        }
        if (!g_uploadReady) {
            request->send(400, "text/plain", "Upload failed");
            return;
        }
    }

    if (g_uploadOk) {
        g_uploadReady = false;
        g_uploadOk = false;
        g_uploadPending = false;
        g_uploadProcessing = false;
        g_uploadError.clear();
        request->redirect("/");
        return;
    }

    const String error = g_uploadError.c_str();
    g_uploadReady = false;
    g_uploadOk = false;
    g_uploadPending = false;
    g_uploadProcessing = false;
    g_uploadError.clear();
    request->send(400, "text/plain", error);
}

void handleUploadChunk(AsyncWebServerRequest*, const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
    if (index == 0) {
        g_uploadBuffer.clear();
        g_uploadFilename = filename.c_str();
        g_uploadReady = false;
        g_uploadOk = false;
        g_uploadPending = false;
        g_uploadProcessing = false;
        g_uploadError.clear();
        LOG_INFO(TAG, "start file=%s", g_uploadFilename.c_str());
    }

    g_uploadBuffer.append(reinterpret_cast<const char*>(data), len);
    LOG_DEBUG(TAG, "chunk file=%s index=%u len=%u final=%d total=%u",
              g_uploadFilename.c_str(),
              static_cast<unsigned>(index),
              static_cast<unsigned>(len),
              final ? 1 : 0,
              static_cast<unsigned>(g_uploadBuffer.size()));

    if (!final) {
        return;
    }

    LOG_INFO(TAG, "final file=%s total=%u", g_uploadFilename.c_str(), static_cast<unsigned>(g_uploadBuffer.size()));
    g_uploadPending = true;
}

void processUpload() {
    if (!g_uploadPending || g_uploadProcessing) {
        return;
    }

    g_uploadProcessing = true;
    g_uploadPending = false;

    LOG_INFO(TAG, "processing upload file=%s", g_uploadFilename.c_str());

    rapidjson::Document normalizedDoc;
    std::string normalizeError;
    mg::Result result;
    if (normalizeUpload(g_uploadBuffer, normalizedDoc, normalizeError)) {
        LOG_INFO(TAG, "normalized bytes=%u", static_cast<unsigned>(g_uploadBuffer.size()));
        LOG_INFO(TAG, "processUpload: importing filename=%s", g_uploadFilename.c_str());
        result = mg::importPackage(normalizedDoc);
    } else {
        LOG_ERROR(TAG, "normalize failed: %s", normalizeError.c_str());
        result = mg::Error(normalizeError);
    }

    LOG_INFO(TAG, "import result ok=%d err=%s", result.ok ? 1 : 0, result.ok ? "" : result.error.c_str());
    g_uploadOk = result.ok;
    g_uploadError = result.ok ? std::string() : result.error;
    g_uploadReady = true;
    g_uploadProcessing = false;
    g_uploadBuffer.clear();
    g_uploadFilename.clear();
}

} // namespace

void process() {
    processUpload();
}

void start(mg::ContextId contextId) {
    g_contextId = contextId;

    WiFi.mode(WIFI_AP);
    WiFi.softAP("multigauge");

    server.on("/", HTTP_GET, handleIndex);
    server.on("/show", HTTP_POST, handleShow);
    server.on("/export", HTTP_POST, handleExport);
    server.on("/remove", HTTP_POST, handleRemove);
    server.on("/upload", HTTP_POST, handleUpload, handleUploadChunk);
    server.begin();
}

}
