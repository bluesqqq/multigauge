import { getPageRenderer } from "/multigauge-web/js/pageRenderer.js";

document.addEventListener("DOMContentLoaded", async () => {
    const canvases = [...document.querySelectorAll(".post-canvas")];
    if (!canvases.length) return;

    const renderer = await getPageRenderer((message) => console.info("[gaugePreview]", message));

    for (const canvas of canvases) {
        const postId = String(canvas.id || "").replace(/^gaugeCanvas-/, "");
        const gaugeDataElement = document.getElementById(`gaugeData-${postId}`);
        if (!postId || !gaugeDataElement) continue;

        try {
            await renderer.renderGaugeJson({
                canvas,
                wasmPath: `/work/posts/post-${postId}.package.json`,
                gaugeJson: gaugeDataElement.textContent || "",
                name: `Post ${postId}`,
            });
        } catch (error) {
            console.error(`[gaugePreview] Failed to render post ${postId}:`, error);
        }
    }
});
