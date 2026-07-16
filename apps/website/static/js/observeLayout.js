export function observeLayout(elements, update) {
    const observedElements = elements.filter(Boolean);
    if (!observedElements.length || typeof update !== "function") return null;

    const observer = new ResizeObserver(update);

    for (const element of observedElements) {
        observer.observe(element);
    }

    window.addEventListener("load", update, { once: true });
    update();
    return observer;
}
