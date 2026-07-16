import { loadGaugeRenderer } from "/static/js/gaugeRenderer.js";

document.addEventListener("DOMContentLoaded", () => {
  const fileInput = document.querySelector('input[name="package"]');
  const canvas = document.getElementById("gaugeCanvas");
  if (!fileInput || !canvas) return;

  let currentViewId = 0;

  fileInput.addEventListener("change", async (event) => {
    const file = event.target.files?.[0];
    if (!file) return;

    if (!file.name.match(/\.(json|package)$/i)) {
      alert("Please select a valid package JSON file");
      event.target.value = "";
      return;
    }

    try {
      const fileContent = await file.text();
      const packageData = JSON.parse(fileContent);
      if (!packageData || !Array.isArray(packageData.faces) || !packageData.faces.length || !packageData.faces[0]?.face) {
        throw new Error("Package is missing face data.");
      }

      const renderer = await loadGaugeRenderer("upload");
      if (currentViewId) {
        renderer.removeView(currentViewId);
        currentViewId = 0;
      }

      const result = await renderer.renderGaugeText({
        canvas,
        wasmPath: "/work/uploads/upload-preview.package.json",
        gaugeText: JSON.stringify(packageData.faces[0].face),
        name: packageData.name || file.name,
      });

      currentViewId = result.id >>> 0;
    } catch (error) {
      console.error("[upload] Failed to preview package file:", error);
      alert("Invalid package file format.");
    }
  });
});
