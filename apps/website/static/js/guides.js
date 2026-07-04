export function setSvgViewBox(svg, width, height) {
    svg.setAttribute("viewBox", `0 0 ${width} ${height}`);
}

export function setLine(line, x1, y1, x2, y2) {
    if (!line) return;

    line.setAttribute("x1", x1);
    line.setAttribute("y1", y1);
    line.setAttribute("x2", x2);
    line.setAttribute("y2", y2);
}

export function setCrosshair(crosshair, x, y) {
    if (!crosshair) return;

    crosshair.setAttribute("x", x);
    crosshair.setAttribute("y", y);
}

export function createSvgElement(svg, name, attributes = {}) {
    const element = document.createElementNS("http://www.w3.org/2000/svg", name);

    for (const [key, value] of Object.entries(attributes)) {
        element.setAttribute(key, value);
    }

    svg.appendChild(element);
    return element;
}

export function createGuideLine(svg, className = "") {
    const attrs = className ? { class: className } : {};
    return createSvgElement(svg, "line", attrs);
}

export function createGuideCrosshair(svg, href, className = "") {
    const attrs = { href };

    if (className) {
        attrs.class = className;
    }

    return createSvgElement(svg, "use", attrs);
}

export function drawHorizontalLine(
    { line, startCrosshair, endCrosshair },
    y,
    x1,
    x2,
    startCrosshairX = x1,
    endCrosshairX = x2
) {
    setLine(line, x1, y, x2, y);
    setCrosshair(startCrosshair, startCrosshairX, y);
    setCrosshair(endCrosshair, endCrosshairX, y);
}

export function drawVerticalLine(
    { line, startCrosshair, endCrosshair },
    x,
    y1,
    y2,
    startCrosshairY = y1,
    endCrosshairY = y2
) {
    setLine(line, x, y1, x, y2);
    setCrosshair(startCrosshair, x, startCrosshairY);
    setCrosshair(endCrosshair, x, endCrosshairY);
}

export function drawLine(line, x1, y1, x2, y2) {
    setLine(line, x1, y1, x2, y2);
}

export function drawBoxGuides({ svg, box, lines = {}, crosshairs = {}, extendToContainer = false }) {
    const { x = 0, y = 0, width = 0, height = 0 } = box ?? {};
    const right = x + width;
    const bottom = y + height;
    const viewBox = svg?.viewBox?.baseVal;
    const contextWidth = viewBox?.width || svg?.clientWidth || right;
    const contextHeight = viewBox?.height || svg?.clientHeight || bottom;
    const horizontalStart = extendToContainer ? 0 : x;
    const horizontalEnd = extendToContainer ? contextWidth : right;
    const verticalStart = extendToContainer ? 0 : y;
    const verticalEnd = extendToContainer ? contextHeight : bottom;

    drawLine(lines.top, horizontalStart, y, horizontalEnd, y);
    drawLine(lines.bottom, horizontalStart, bottom, horizontalEnd, bottom);
    drawLine(lines.left, x, verticalStart, x, verticalEnd);
    drawLine(lines.right, right, verticalStart, right, verticalEnd);

    setCrosshair(crosshairs.topLeft, x, y);
    setCrosshair(crosshairs.topRight, right, y);
    setCrosshair(crosshairs.bottomLeft, x, bottom);
    setCrosshair(crosshairs.bottomRight, right, bottom);
}

export function createBoxGuideLayer(svg, { crosshairHref, lineClass = "", crosshairClass = "" } = {}) {
    const lines = {
        top: createGuideLine(svg, lineClass),
        bottom: createGuideLine(svg, lineClass),
        left: createGuideLine(svg, lineClass),
        right: createGuideLine(svg, lineClass),
    };

    const crosshairs = {
        topLeft: createGuideCrosshair(svg, crosshairHref, crosshairClass),
        topRight: createGuideCrosshair(svg, crosshairHref, crosshairClass),
        bottomLeft: createGuideCrosshair(svg, crosshairHref, crosshairClass),
        bottomRight: createGuideCrosshair(svg, crosshairHref, crosshairClass),
    };

    return {
        lines,
        crosshairs,
        update({ box, extendToContainer = false, viewportWidth, viewportHeight } = {}) {
            if (viewportWidth && viewportHeight) {
                setSvgViewBox(svg, viewportWidth, viewportHeight);
            }

            drawBoxGuides({
                svg,
                box,
                lines,
                crosshairs,
                extendToContainer,
            });
        },
    };
}

export function createLineGuideLayer(svg, { crosshairHref, lineClass = "", crosshairClass = "" } = {}) {
    const line = createGuideLine(svg, lineClass);
    const startCrosshair = createGuideCrosshair(svg, crosshairHref, crosshairClass);
    const endCrosshair = createGuideCrosshair(svg, crosshairHref, crosshairClass);

    return {
        line,
        startCrosshair,
        endCrosshair,
        update({
            viewportWidth,
            viewportHeight,
            x1,
            y1,
            x2,
            y2,
            startCrosshairX = x1,
            startCrosshairY = y1,
            endCrosshairX = x2,
            endCrosshairY = y2,
        }) {
            if (viewportWidth && viewportHeight) {
                setSvgViewBox(svg, viewportWidth, viewportHeight);
            }

            setLine(line, x1, y1, x2, y2);
            setCrosshair(startCrosshair, startCrosshairX, startCrosshairY);
            setCrosshair(endCrosshair, endCrosshairX, endCrosshairY);
        },
    };
}
