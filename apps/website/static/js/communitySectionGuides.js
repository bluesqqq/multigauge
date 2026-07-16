import { createBoxGuideLayer } from "/static/js/guides.js";
import { observeLayout } from "/static/js/observeLayout.js";

export function initCommunitySectionGuides() {
    const section = document.querySelector(".community-section");
    const inner = document.querySelector(".community-section-inner");
    const svg = document.querySelector(".community-section-guides svg");

    if (!section || !inner || !svg) return;

    const boxGuide = createBoxGuideLayer(svg, {
        crosshairHref: "#community-section-crosshair",
    });

    const update = () => {
        const sectionRect = section.getBoundingClientRect();
        const innerRect = inner.getBoundingClientRect();
        const { width, height } = sectionRect;

        if (!width || !height) return;

        boxGuide.update({
            box: {
                x: innerRect.left - sectionRect.left,
                y: innerRect.top - sectionRect.top,
                width: innerRect.width,
                height: innerRect.height,
            },
            extendToContainer: true,
            viewportWidth: width,
            viewportHeight: height,
        });
    };

    observeLayout([section, inner], update);
}

document.addEventListener("DOMContentLoaded", initCommunitySectionGuides);
