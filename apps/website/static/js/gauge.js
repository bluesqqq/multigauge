import { loadGaugeRenderer } from "/static/js/gaugeRenderer.js";

document.addEventListener("DOMContentLoaded", async () => {
    const canvas = document.getElementById("gaugeCanvas");
    const gaugeDataElement = document.getElementById("post-gauge-data");
    if (!canvas || !gaugeDataElement) return;

    try {
        const renderer = await loadGaugeRenderer("gauge");
        await renderer.renderGaugeJson({
            canvas,
            wasmPath: "/work/posts/post-current.package.json",
            gaugeJson: gaugeDataElement.textContent || "",
            name: "Post Preview",
        });
    } catch (error) {
        console.error("[gauge] Failed to render post preview:", error);
    }
});
