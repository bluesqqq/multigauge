import { loadGaugeRenderer } from "/static/js/gaugeRenderer.js";

const PREVIEW_PACKAGE_URL = "/static/json/designer-preview.package.json";

const COLOR_PALETTES = {
    red: {
        "#ff4d2e": "#ff4d2e",
        "#ffb39a": "#ffb39a",
        "#6b7280": "#6b7280",
        "#111114": "#111114",
        "#1f2937": "#1f2937",
        "#0b0f14": "#0b0f14",
        "#07111b": "#07111b",
        "#0f172a": "#0f172a",
        "#ffffff": "#ffffff",
        "#ffd8a8": "#ffd8a8",
    },
    yellow: {
        "#ff4d2e": "#f59e0b",
        "#ffb39a": "#fde68a",
        "#6b7280": "#a16207",
        "#111114": "#111114",
        "#1f2937": "#292524",
        "#0b0f14": "#110f0a",
        "#07111b": "#0f0d08",
        "#0f172a": "#1f1b11",
        "#ffffff": "#ffffff",
        "#ffd8a8": "#fff1b2",
    },
    green: {
        "#ff4d2e": "#22c55e",
        "#ffb39a": "#86efac",
        "#6b7280": "#15803d",
        "#111114": "#111114",
        "#1f2937": "#1f2937",
        "#0b0f14": "#0a120d",
        "#07111b": "#07150d",
        "#0f172a": "#102013",
        "#ffffff": "#ffffff",
        "#ffd8a8": "#d9f99d",
    },
    blue: {
        "#ff4d2e": "#3b82f6",
        "#ffb39a": "#93c5fd",
        "#6b7280": "#1d4ed8",
        "#111114": "#111114",
        "#1f2937": "#1f2937",
        "#0b0f14": "#0b1018",
        "#07111b": "#071420",
        "#0f172a": "#101b2f",
        "#ffffff": "#ffffff",
        "#ffd8a8": "#bfdbfe",
    },
};

function setActiveTab(tablist, activeTab) {
    const tabs = tablist.querySelectorAll(".designer-preview-tab");

    for (const tab of tabs) {
        const isActive = tab === activeTab;
        tab.setAttribute("aria-selected", isActive ? "true" : "false");
        tab.tabIndex = isActive ? 0 : -1;
    }
}

function initTablist(tablist, onChange) {
    const tabs = Array.from(tablist.querySelectorAll(".designer-preview-tab"));
    if (!tabs.length) return;

    const initialActive = tabs.find((tab) => tab.getAttribute("aria-selected") === "true") ?? tabs[0];
    setActiveTab(tablist, initialActive);
    onChange?.(tablist.dataset.designerGroup, initialActive);

    tablist.addEventListener("click", (event) => {
        const tab = event.target.closest(".designer-preview-tab");
        if (!tab || !tablist.contains(tab)) return;

        setActiveTab(tablist, tab);
        onChange?.(tablist.dataset.designerGroup, tab);
    });

    tablist.addEventListener("keydown", (event) => {
        if (!["ArrowLeft", "ArrowRight", "Home", "End"].includes(event.key)) return;

        const currentIndex = tabs.findIndex((tab) => tab.getAttribute("aria-selected") === "true");
        if (currentIndex < 0) return;

        let nextIndex = currentIndex;
        if (event.key === "ArrowLeft") {
            nextIndex = (currentIndex - 1 + tabs.length) % tabs.length;
        } else if (event.key === "ArrowRight") {
            nextIndex = (currentIndex + 1) % tabs.length;
        } else if (event.key === "Home") {
            nextIndex = 0;
        } else if (event.key === "End") {
            nextIndex = tabs.length - 1;
        }

        event.preventDefault();
        const nextTab = tabs[nextIndex];
        setActiveTab(tablist, nextTab);
        onChange?.(tablist.dataset.designerGroup, nextTab);
        nextTab.focus();
    });
}

function deepReplaceColors(value, palette) {
    if (Array.isArray(value)) {
        return value.map((entry) => deepReplaceColors(entry, palette));
    }

    if (!value || typeof value !== "object") {
        if (typeof value === "string" && palette[value.toLowerCase()]) {
            return palette[value.toLowerCase()];
        }
        return value;
    }

    const out = {};
    for (const [key, child] of Object.entries(value)) {
        out[key] = deepReplaceColors(child, palette);
    }
    return out;
}

function themeFace(face, paletteKey) {
    const palette = COLOR_PALETTES[paletteKey] ?? COLOR_PALETTES.red;
    return deepReplaceColors(structuredClone(face), palette);
}

async function loadPreviewPackage() {
    const response = await fetch(PREVIEW_PACKAGE_URL, { cache: "no-store" });
    if (!response.ok) {
        throw new Error(`Failed to load designer preview package (${response.status}).`);
    }

    return response.json();
}

function resizeCanvasToElement(canvas, element) {
    const rect = element.getBoundingClientRect();
    const width = Math.max(1, Math.round(rect.width));
    const height = Math.max(1, Math.round(rect.height));

    canvas.width = width;
    canvas.height = height;

    return { width, height };
}

document.addEventListener("DOMContentLoaded", async () => {
    const canvas = document.getElementById("designerPreviewCanvas");
    const ring = canvas?.closest(".designer-preview-ring");
    const faceTablist = document.querySelector('[data-designer-group="face"]');
    const colorTablist = document.querySelector('[data-designer-group="color"]');

    if (!canvas || !ring || !faceTablist || !colorTablist) return;

    let packageData;
    try {
        packageData = await loadPreviewPackage();
    } catch (error) {
        console.error("[designerPreview] Failed to load preview package:", error);
        return;
    }

    const faces = Array.isArray(packageData.faces) ? packageData.faces : [];
    if (!faces.length) {
        console.error("[designerPreview] Preview package contains no faces.");
        return;
    }

    const facesByKey = new Map(faces.map((entry) => [String(entry.name || "").toUpperCase(), entry]));
    let activeFaceKey = "RPM";
    let activeColorKey = "red";
    let currentViewId = 0;
    let renderStamp = 0;

    const renderer = await loadGaugeRenderer("designerPreview");

    const renderCurrent = async () => {
        const faceEntry = facesByKey.get(activeFaceKey) ?? faces[0];
        if (!faceEntry?.face) return;

        const stamp = ++renderStamp;
        const { width, height } = resizeCanvasToElement(canvas, ring);
        const themedFace = themeFace(faceEntry.face, activeColorKey);

        if (currentViewId) {
            renderer.removeView(currentViewId);
            currentViewId = 0;
        }

        try {
            const result = await renderer.renderGaugeJson({
                canvas,
                gaugeJson: themedFace,
                name: `${faceEntry.name || "Gauge"} Preview`,
                width,
                height,
            });

            if (stamp !== renderStamp) {
                renderer.removeView(result.id);
                return;
            }

            currentViewId = result.id;
        } catch (error) {
            console.error("[designerPreview] Failed to render preview:", error);
        }
    };

    const updateSelectedFace = (_group, tab) => {
        if (tab.dataset.faceKey) {
            activeFaceKey = tab.dataset.faceKey.toUpperCase();
        }
        if (tab.dataset.colorKey) {
            activeColorKey = tab.dataset.colorKey.toLowerCase();
        }
        renderCurrent();
    };

    initTablist(faceTablist, updateSelectedFace);
    initTablist(colorTablist, updateSelectedFace);

    const resizeObserver = new ResizeObserver(() => {
        renderCurrent();
    });
    resizeObserver.observe(ring);

    await renderCurrent();
});
