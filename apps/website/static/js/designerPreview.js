function setActiveTab(tablist, activeTab) {
    const tabs = tablist.querySelectorAll(".designer-preview-tab");

    for (const tab of tabs) {
        const isActive = tab === activeTab;
        tab.setAttribute("aria-selected", isActive ? "true" : "false");
        tab.tabIndex = isActive ? 0 : -1;
    }
}

function initTablist(tablist) {
    const tabs = Array.from(tablist.querySelectorAll(".designer-preview-tab"));

    if (!tabs.length) return;

    const initialActive = tabs.find((tab) => tab.getAttribute("aria-selected") === "true") ?? tabs[0];
    setActiveTab(tablist, initialActive);

    tablist.addEventListener("click", (event) => {
        const tab = event.target.closest(".designer-preview-tab");
        if (!tab || !tablist.contains(tab)) return;

        setActiveTab(tablist, tab);
    });

    tablist.addEventListener("keydown", (event) => {
        if (!["ArrowLeft", "ArrowRight", "Home", "End"].includes(event.key)) return;

        const currentIndex = tabs.findIndex((tab) => tab.getAttribute("aria-selected") === "true");
        if (currentIndex < 0) return;

        let nextIndex = currentIndex;

        if (event.key === "ArrowLeft") {
            nextIndex = (currentIndex - 1 + tabs.length) % tabs.length;
        } else if (event.key === "ArrowRight") {
            nextIndex = (currentIndex + 1) % tabs.length;
        } else if (event.key === "Home") {
            nextIndex = 0;
        } else if (event.key === "End") {
            nextIndex = tabs.length - 1;
        }

        event.preventDefault();
        const nextTab = tabs[nextIndex];
        setActiveTab(tablist, nextTab);
        nextTab.focus();
    });
}

function initDesignerPreview() {
    const tablists = document.querySelectorAll(".designer-preview-tabs");

    for (const tablist of tablists) {
        initTablist(tablist);
    }
}

document.addEventListener("DOMContentLoaded", initDesignerPreview);
