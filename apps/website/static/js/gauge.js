import { getPageRenderer } from "/multigauge-web/js/pageRenderer.js";

document.addEventListener("DOMContentLoaded", async () => {
    const canvas = document.getElementById("gaugeCanvas");
    if (!canvas || typeof postData === "undefined") return;

    try {
        const renderer = await getPageRenderer((message) => console.info("[gauge]", message));
        await renderer.renderGaugeJson({
            canvas,
            wasmPath: "/work/posts/post-current.package.json",
            gaugeJson: postData,
            name: "Post Preview",
        });
    } catch (error) {
        console.error("[gauge] Failed to render post preview:", error);
    }
});
