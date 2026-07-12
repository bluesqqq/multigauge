import { getPageRenderer } from "/multigauge-web/js/pageRenderer.js";

const rendererCache = new Map();

export function loadGaugeRenderer(prefix = "gauge") {
    if (!rendererCache.has(prefix)) {
        rendererCache.set(prefix, getPageRenderer((message) => console.info(`[${prefix}]`, message)));
    }

    return rendererCache.get(prefix);
}
