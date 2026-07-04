import {
    createBoxGuideLayer,
    createLineGuideLayer,
} from "/static/js/guides.js";

export function initProductCardGuides() {
    const card = document.querySelector(".product-card");
    const inner = document.querySelector(".product-card-inner");
    const svg = document.querySelector(".product-card-guides svg");

    if (!card || !inner || !svg) return;

    const boxGuide = createBoxGuideLayer(svg, {
        crosshairHref: "#product-card-crosshair",
    });
    const centerGuide = createLineGuideLayer(svg, {
        crosshairHref: "#product-card-crosshair",
    });

    const update = () => {
        const cardRect = card.getBoundingClientRect();
        const innerRect = inner.getBoundingClientRect();
        const { width, height } = cardRect;

        if (!width || !height) return;

        const boxX = innerRect.left - cardRect.left;
        const boxY = innerRect.top - cardRect.top;
        const boxWidth = innerRect.width;
        const boxHeight = innerRect.height;
        const topY = boxY;
        const bottomY = boxY + boxHeight;
        const crosshairHalfHeight = 6;
        const centerTopY = topY + 16 + crosshairHalfHeight;
        const centerBottomY = bottomY - 16 - crosshairHalfHeight;
        const centerX = width / 2;

        boxGuide.update({
            box: { x: boxX, y: boxY, width: boxWidth, height: boxHeight },
            extendToContainer: true,
            viewportWidth: width,
            viewportHeight: height,
        });

        centerGuide.update({
            viewportWidth: width,
            viewportHeight: height,
            x1: centerX,
            y1: centerTopY,
            x2: centerX,
            y2: centerBottomY,
        });
    };

    const observer = new ResizeObserver(update);
    observer.observe(card);
    observer.observe(inner);
    window.addEventListener("load", update, { once: true });
    update();
}

document.addEventListener("DOMContentLoaded", initProductCardGuides);
