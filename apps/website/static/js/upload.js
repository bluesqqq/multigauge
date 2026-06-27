import { getPageRenderer } from "/multigauge-web/js/pageRenderer.js";

document.addEventListener("DOMContentLoaded", () => {
  const fileInput = document.querySelector('input[name="gauge"]');
  const canvas = document.getElementById("gaugeCanvas");
  if (!fileInput || !canvas) return;

  let currentViewId = 0;

  fileInput.addEventListener("change", async (event) => {
    const file = event.target.files?.[0];
    if (!file) return;

    if (!file.name.endsWith(".gauge")) {
      alert("Please select a valid .gauge file");
      event.target.value = "";
      return;
    }

    try {
      const fileContent = await file.text();
      JSON.parse(fileContent);

      const renderer = await getPageRenderer((message) => console.info("[upload]", message));
      if (currentViewId) {
        renderer.removeView(currentViewId);
        currentViewId = 0;
      }

      const result = await renderer.renderGaugeText({
        canvas,
        wasmPath: "/work/uploads/upload-preview.gauge",
        gaugeText: fileContent,
        name: file.name,
      });

      currentViewId = result.id >>> 0;
    } catch (error) {
      console.error("[upload] Failed to preview gauge file:", error);
      alert("Invalid gauge file format.");
    }
  });
});
