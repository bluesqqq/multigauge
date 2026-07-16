const DROPDOWN_OVERLAY_ID = "mgDropdownLayer";
let dropdownOverlay = null;
const openDropdownStack = [];
const DROPDOWN_MARGIN = 8;
const stickySubmenuControllers = new WeakMap();

function getDropdownOverlay() {
    if (dropdownOverlay && document.contains(dropdownOverlay)) {
        return dropdownOverlay;
    }

    dropdownOverlay = document.getElementById(DROPDOWN_OVERLAY_ID);
    if (dropdownOverlay) {
        return dropdownOverlay;
    }

    dropdownOverlay = document.createElement("div");
    dropdownOverlay.id = DROPDOWN_OVERLAY_ID;
    dropdownOverlay.className = "dropdownOverlay";

    const mountPoint = document.body || document.documentElement;
    mountPoint.appendChild(dropdownOverlay);
    return dropdownOverlay;
}

function isScrollableOverflow(value) {
    return value === "auto" || value === "scroll";
}

function getViewportRect() {
    return {
        left: 0,
        top: 0,
        right: window.innerWidth,
        bottom: window.innerHeight,
        width: window.innerWidth,
        height: window.innerHeight,
    };
}

function normalizeRect(rect) {
    if (!rect) return getViewportRect();

    const left = Number.isFinite(rect.left) ? rect.left : 0;
    const top = Number.isFinite(rect.top) ? rect.top : 0;
    const right = Number.isFinite(rect.right) ? rect.right : left + (rect.width || 0);
    const bottom = Number.isFinite(rect.bottom) ? rect.bottom : top + (rect.height || 0);

    return {
        left,
        top,
        right,
        bottom,
        width: Math.max(0, right - left),
        height: Math.max(0, bottom - top),
    };
}

function clamp(value, min, max) {
    if (max < min) return min;
    return Math.max(min, Math.min(value, max));
}

function getBoundaryRect(anchorEl, explicitBoundaryRect = null) {
    if (explicitBoundaryRect) {
        return normalizeRect(explicitBoundaryRect);
    }

    if (anchorEl instanceof Element) {
        let current = anchorEl.parentElement;

        while (current) {
            if (current === document.body || current === document.documentElement) {
                break;
            }

            const style = getComputedStyle(current);
            const clipsX = isScrollableOverflow(style.overflowX) || isScrollableOverflow(style.overflow);
            const clipsY = isScrollableOverflow(style.overflowY) || isScrollableOverflow(style.overflow);

            if (clipsX || clipsY) {
                return normalizeRect(current.getBoundingClientRect());
            }

            current = current.parentElement;
        }
    }

    return getViewportRect();
}

function getAnchorRect(anchorEl, left, top) {
    if (anchorEl instanceof Element) {
        return normalizeRect(anchorEl.getBoundingClientRect());
    }

    return normalizeRect({
        left,
        right: left,
        top,
        bottom: top,
        width: 0,
        height: 0,
    });
}

function measureDropdown(dropdownList, boundaryRect) {
    const maxWidth = Math.max(0, Math.floor(boundaryRect.width - DROPDOWN_MARGIN * 2));
    const maxHeight = Math.max(0, Math.floor(boundaryRect.height - DROPDOWN_MARGIN * 2));

    dropdownList.style.width = "";
    dropdownList.style.maxWidth = `${maxWidth}px`;
    dropdownList.style.maxHeight = `${maxHeight}px`;

    const naturalWidth = Math.max(dropdownList.scrollWidth, dropdownList.offsetWidth, 180);
    const finalWidth = Math.min(naturalWidth, maxWidth);

    dropdownList.style.width = `${Math.max(0, finalWidth)}px`;

    return {
        width: dropdownList.offsetWidth,
        height: dropdownList.offsetHeight,
    };
}

function positionDropdown(dropdownList, {
    anchorEl = null,
    left = 0,
    top = 0,
    boundaryRect = null,
    placement = "bottom-start",
} = {}) {
    const boundary = getBoundaryRect(anchorEl, boundaryRect);
    const anchor = getAnchorRect(anchorEl, left, top);
    const size = measureDropdown(dropdownList, boundary);
    const minLeft = boundary.left + DROPDOWN_MARGIN;
    const maxLeft = boundary.right - DROPDOWN_MARGIN - size.width;
    const minTop = boundary.top + DROPDOWN_MARGIN;
    const maxTop = boundary.bottom - DROPDOWN_MARGIN - size.height;
    let resolvedLeft = anchor.left;
    let resolvedTop = anchor.bottom;

    if (placement === "right-start") {
        const roomRight = boundary.right - anchor.right - DROPDOWN_MARGIN;
        const roomLeft = anchor.left - boundary.left - DROPDOWN_MARGIN;
        const openRight = roomRight >= size.width || roomRight >= roomLeft;

        resolvedLeft = openRight ? anchor.right : anchor.left - size.width;
        resolvedTop = clamp(anchor.top, minTop, maxTop);
    } else {
        const roomBelow = boundary.bottom - anchor.bottom - DROPDOWN_MARGIN;
        const roomAbove = anchor.top - boundary.top - DROPDOWN_MARGIN;
        const openBelow = roomBelow >= size.height || roomBelow >= roomAbove;

        resolvedLeft = anchor.left;
        resolvedTop = openBelow ? anchor.bottom : anchor.top - size.height;
    }

    dropdownList.style.left = `${clamp(resolvedLeft, minLeft, maxLeft)}px`;
    dropdownList.style.top = `${clamp(resolvedTop, minTop, maxTop)}px`;
    dropdownList._dropdownBoundaryRect = boundary;
}

function closeAllDropdowns() {
    const overlay = getDropdownOverlay();
    if (!overlay) return;
    overlay.innerHTML = "";
    overlay.classList.remove("locked");
    openDropdownStack.length = 0;
}

export { closeAllDropdowns };

function isDividerOption(option) {
    return !option || typeof option !== "object" || Object.keys(option).length === 0 || option.type === "divider";
}

function cleanMenuOptions(options) {
    const cleaned = [];

    for (const option of options) {
        if (isDividerOption(option)) {
            if (!cleaned.length || isDividerOption(cleaned[cleaned.length - 1])) {
                continue;
            }
            cleaned.push({});
            continue;
        }

        cleaned.push(option);
    }

    while (cleaned.length && isDividerOption(cleaned[0])) {
        cleaned.shift();
    }

    while (cleaned.length && isDividerOption(cleaned[cleaned.length - 1])) {
        cleaned.pop();
    }

    return cleaned;
}

function normalizeSearchConfig(search, config = {}) {
    const candidate = search || config.search || null;
    if (!candidate) return null;

    if (candidate === true) {
        return {
            placeholder: "Search...",
            autofocus: true,
        };
    }

    if (typeof candidate === "string") {
        return {
            placeholder: candidate,
            autofocus: true,
        };
    }

    if (candidate && typeof candidate === "object") {
        return {
            placeholder: "Search...",
            autofocus: true,
            ...candidate,
        };
    }

    return null;
}

function getOptionSearchText(option) {
    if (!option || typeof option !== "object") return "";

    const chunks = [];
    const values = [
        option.searchText,
        option.name,
        option.label,
        option.context,
        option.value,
        option.description,
        ...(Array.isArray(option.searchTerms) ? option.searchTerms : []),
    ];

    for (const value of values) {
        if (typeof value === "string" && value.trim()) {
            chunks.push(value.trim());
        }
    }

    return chunks.join(" ").toLowerCase();
}

function filterMenuOptions(options, query, searchConfig = null) {
    const normalizedQuery = String(query || "").trim().toLowerCase();
    if (!normalizedQuery) {
        return cleanMenuOptions(options);
    }

    const matcher = typeof searchConfig?.matcher === "function" ? searchConfig.matcher : null;
    const filtered = [];

    for (const option of options) {
        if (isDividerOption(option)) {
            filtered.push({});
            continue;
        }

        const submenu = Array.isArray(option.options) ? option.options : null;
        const matches = matcher
            ? !!matcher(option, normalizedQuery)
            : getOptionSearchText(option).includes(normalizedQuery);

        if (submenu) {
            const filteredSubmenu = filterMenuOptions(submenu, normalizedQuery, searchConfig);
            if (matches) {
                filtered.push({
                    ...option,
                    options: submenu,
                });
            } else if (filteredSubmenu.length) {
                filtered.push({
                    ...option,
                    options: filteredSubmenu,
                });
            }
            continue;
        }

        if (matches) {
            filtered.push(option);
        }
    }

    return cleanMenuOptions(filtered);
}

function createItem(name, context = null, disabled = null, selected = false, dimmed = false) {
    const item = document.createElement("li");
    item.className = "dropdown-item";

    if (disabled === true || dimmed === true) item.classList.add("disabled");
    if (selected === true) item.classList.add("selected");
    
    const nameSpan = document.createElement("span");
    nameSpan.textContent = name;
    item.appendChild(nameSpan);

    if (context != null && typeof context === 'string') {
        const contextSpan = document.createElement("span");
        contextSpan.className = "dropdown-item-context";
        contextSpan.textContent = context;
        item.appendChild(contextSpan);
    }

    return item;
}

export function openDropdown(options, left, top, onSelect = null, first = true, search = false, config = {}) {
    const overlay = getDropdownOverlay();
    if (!overlay) return null;

    if (first) {
        closeAllDropdowns();
        overlay.classList.add("locked");
    }

    const dropdownList = document.createElement("ul");
    dropdownList.className = "dropdown-list";
    dropdownList.addEventListener("click", (e) => e.stopPropagation());
    overlay.appendChild(dropdownList);

    const normalizedOptions = Array.isArray(options) ? options.map(normalizeMenuItem) : [];
    const searchConfig = normalizeSearchConfig(search, config);
    const hasSearch = !!searchConfig;
    const contentStartIndex = hasSearch ? 1 : 0;

    openDropdownStack.push(dropdownList);

    const clearRenderedItems = () => {
        while (dropdownList.children.length > contentStartIndex) {
            dropdownList.removeChild(dropdownList.lastChild);
        }
    };

    const renderOptions = (filteredOptions) => {
        clearRenderedItems();

        if (!filteredOptions.length) {
            const emptyItem = document.createElement("li");
            emptyItem.className = "dropdown-empty";
            emptyItem.textContent = searchConfig?.emptyText || "No results";
            dropdownList.appendChild(emptyItem);
            positionDropdown(dropdownList, { ...config, left, top });
            return;
        }

        filteredOptions.forEach((option) => {
            addDropdownItem(option, dropdownList, onSelect);
        });
        positionDropdown(dropdownList, { ...config, left, top });
    };

    if (hasSearch) {
        const searchWrap = document.createElement("li");
        searchWrap.className = "dropdown-search-wrap";

        const searchInput = document.createElement("input");
        searchInput.type = "text";
        searchInput.placeholder = searchConfig.placeholder || "Search...";
        searchInput.className = "dropdown-search";
        searchWrap.appendChild(searchInput);
        dropdownList.appendChild(searchWrap);

        const applyFilter = () => {
            const filtered = filterMenuOptions(normalizedOptions, searchInput.value, searchConfig);
            renderOptions(filtered);
        };

        searchInput.addEventListener("input", applyFilter);
        searchInput.addEventListener("keydown", (event) => {
            if (event.key === "Escape") {
                event.stopPropagation();
                event.preventDefault();
                closeAllDropdowns();
            }
        });

        applyFilter();

        if (searchConfig.autofocus !== false) {
            requestAnimationFrame(() => {
                searchInput.focus();
                searchInput.select();
            });
        }
    } else {
        normalizedOptions.forEach((option) => {
            addDropdownItem(option, dropdownList, onSelect);
        });
        positionDropdown(dropdownList, { ...config, left, top });
    }

    return dropdownList;
}

function removeDropdownsDeeperThan(targetDropdown) {
    const targetIndex = openDropdownStack.indexOf(targetDropdown);
    const overlay = getDropdownOverlay();
    while (openDropdownStack.length > targetIndex + 1) {
        const removed = openDropdownStack.pop();
        if (removed?.parentNode === overlay) {
            overlay.removeChild(removed);
        }
    }
}

function addDivider(dropdownList) {
    const divider = document.createElement("li");
    divider.className = "dropdown-divider";
    dropdownList.appendChild(divider);
}

function addDropdownItem(option, dropdownList, onSelect = null) {
    if (isDividerOption(option)) { addDivider(dropdownList); return; }

    const name     = option.name || option.label || "NO NAME";
    const context  = option.context;
    const disabled = option.disabled || false;
    const selected = option.selected || false;
    const dimmed = option.dimmed || false;
    const optionSubmenu = Array.isArray(option.options) ? option.options : Array.isArray(option.submenu) ? option.submenu : null;

    if (optionSubmenu) { // Sub-options
        const nestedItem = createItem(name, ">", disabled, selected, dimmed);
        dropdownList.appendChild(nestedItem);

        nestedItem.addEventListener("mouseenter", () => {
            const stickyItem = dropdownList.querySelector(".stuck");

            if (stickyItem && stickyItem !== nestedItem) {
                stickyItem.classList.remove("stuck");
            }
            
            nestedItem.classList.add("stuck");

            removeDropdownsDeeperThan(dropdownList);

            const rect = nestedItem.getBoundingClientRect();
            openDropdown(optionSubmenu, rect.right, rect.top, onSelect, false, option.search || option.searchable || false, {
                anchorEl: nestedItem,
                boundaryRect: dropdownList._dropdownBoundaryRect || null,
                placement: "right-start",
            });
        });
    } else { // Option
        const item = createItem(name, context, disabled, selected, dimmed);
        dropdownList.appendChild(item);

        if (!disabled) {
            item.addEventListener("mouseenter", () => {
                const stickyItem = dropdownList.querySelector(".stuck");
                if (stickyItem) stickyItem.classList.remove("stuck");

                removeDropdownsDeeperThan(dropdownList);
            });

            item.addEventListener("click", () => {
                closeAllDropdowns();

                // Call both the menu and option's onSelect functions
                if (onSelect && typeof onSelect === 'function') onSelect(option);
                const action = option.onSelect || option.onClick;
                if (action && typeof action === 'function') action();
            });   
        }
    }
}

function normalizeMenuItem(option) {
    if (!option || typeof option !== "object") return option;
    if (isDividerOption(option)) return {};

    const submenu = Array.isArray(option.options) ? option.options : Array.isArray(option.submenu) ? option.submenu : null;
    return {
        ...option,
        name: option.name || option.label || "NO NAME",
        options: submenu ? submenu.map(normalizeMenuItem) : undefined,
        onSelect: option.onSelect || option.onClick || null,
        search: option.search || (option.searchable ? true : null),
    };
}

export function openMenu(items, left, top, config = {}) {
    const normalized = Array.isArray(items) ? items.map(normalizeMenuItem) : [];
    return openDropdown(normalized, left, top, null, true, config.search || false, config);
}

/* 
    Options can have the following properties:

    name       - Displayed name
    context    - Optional displayed context (i.e. "Ctrl+X" or "Del")
    onSelect() - Callback function ran on selection of this option
    options    - Array of nested options that appears in another dropdown on hover
    value      - value this option represent, whether it be a index, object, e.t.c
    disabled   - whether the option should be selectable (also affect dimmed)
    dimmed     - whether the option should be greyed out
    selected   - whether the option is selected or not (for select menus)

    Empty option just draws a divider
*/

export function makeActionMenu(options, labelText = "Actions") {
    const dropdown = document.createElement("div");
    dropdown.className = "dropdown";

    const dropdownButton = document.createElement("div");
    dropdownButton.className = "dropdown-button";
    dropdown.appendChild(dropdownButton);

    const dropdownText = document.createElement("span");
    dropdownText.textContent = labelText;
    dropdownButton.appendChild(dropdownText);

    const dropdownCaret = document.createElement("span");
    dropdownCaret.textContent = "▽";
    dropdownButton.appendChild(dropdownCaret);

    dropdownButton.addEventListener("click", () => {
        const rect = dropdownButton.getBoundingClientRect();

        openDropdown(options, rect.left + window.scrollX, rect.bottom + window.scrollY, null, true, false, {
            anchorEl: dropdownButton,
        });
    });

    return dropdown;
}

export function installStickySubmenuBehavior(menuStackEl, config = {}) {
    if (!(menuStackEl instanceof Element)) return null;

    const existing = stickySubmenuControllers.get(menuStackEl);
    if (existing) return existing;

    const openClass = config.openClass || "open";
    const submenuWrapSelector = config.submenuWrapSelector || ".menuSubmenuWrap";

    let activeWrap = null;

    const closeActiveWrap = () => {
        if (!activeWrap) return;
        activeWrap.classList.remove(openClass);
        activeWrap = null;
    };

    const openWrap = (wrap) => {
        if (!(wrap instanceof Element)) return;
        if (wrap === activeWrap) {
            wrap.classList.add(openClass);
            return;
        }

        closeActiveWrap();
        activeWrap = wrap;
        activeWrap.classList.add(openClass);
    };

    const closeWrap = (wrap) => {
        if (!(wrap instanceof Element)) return;
        if (wrap === activeWrap) {
            closeActiveWrap();
            return;
        }
        wrap.classList.remove(openClass);
    };

    const toggleWrap = (wrap) => {
        if (!(wrap instanceof Element)) return;
        if (wrap === activeWrap || wrap.classList.contains(openClass)) {
            closeWrap(wrap);
            return;
        }
        openWrap(wrap);
    };

    const handlePointerOver = (event) => {
        const target = event.target;
        if (!(target instanceof Element)) return;

        const directStack = target.closest(".menuStack");
        if (directStack !== menuStackEl) return;

        const directWrap = target.closest(submenuWrapSelector);
        if (directWrap && directWrap.parentElement === menuStackEl) {
            openWrap(directWrap);
            return;
        }

        const directButton = target.closest("button");
        if (directButton && directButton.parentElement === menuStackEl) {
            closeActiveWrap();
        }
    };

    menuStackEl.addEventListener("pointerover", handlePointerOver);

    const controller = {
        openSubmenu: openWrap,
        closeSubmenu: closeWrap,
        toggleSubmenu: toggleWrap,
        closeActiveSubmenu: closeActiveWrap,
        clear() {
            closeActiveWrap();
            menuStackEl.querySelectorAll(`${submenuWrapSelector}.${openClass}`).forEach((wrap) => {
                wrap.classList.remove(openClass);
            });
        },
        dispose() {
            menuStackEl.removeEventListener("pointerover", handlePointerOver);
            controller.clear();
            stickySubmenuControllers.delete(menuStackEl);
        },
    };

    stickySubmenuControllers.set(menuStackEl, controller);
    return controller;
}

export function clearStickySubmenuBehavior(rootEl, config = {}) {
    if (!(rootEl instanceof Element)) return;

    const openClass = config.openClass || "open";
    const submenuWrapSelector = config.submenuWrapSelector || ".menuSubmenuWrap";
    rootEl.querySelectorAll(`${submenuWrapSelector}.${openClass}`).forEach((wrap) => {
        wrap.classList.remove(openClass);
    });
}

export function makeSelectMenu(options, labelText = "Select", onChange = null) {
    const dropdown = document.createElement("div");
    dropdown.className = "dropdown";

    const dropdownButton = document.createElement("div");
    dropdownButton.className = "dropdown-button";
    dropdown.appendChild(dropdownButton);

    const dropdownText = document.createElement("span");
    dropdownText.textContent = labelText;
    dropdownButton.appendChild(dropdownText);

    const dropdownCaret = document.createElement("span");
    dropdownCaret.textContent = "▽";
    dropdownButton.appendChild(dropdownCaret);

    const onSelect = (option) => {
        dropdownText.textContent = option.name;

        if (onChange != null && typeof onChange === 'function') onChange(option.value);
    }

    dropdownButton.addEventListener("click", () => {
        const rect = dropdownButton.getBoundingClientRect();

        openDropdown(options, rect.left + window.scrollX, rect.bottom + window.scrollY, onSelect, true, false, {
            anchorEl: dropdownButton,
        });
    });

    return dropdown;
}

const overlay = getDropdownOverlay();
if (overlay != null) {
    overlay.addEventListener('click', () => {
        // Close dropdown if clicked outside
        if (overlay.classList.contains("locked")) {
            closeAllDropdowns();
        }
    });

    overlay.addEventListener('mousedown', (event) => {
        if (overlay.classList.contains("locked") && event.button === 2) {
            closeAllDropdowns();
        }
    });

    window.addEventListener("resize", () => {
        if (overlay.classList.contains("locked")) {
            closeAllDropdowns();
        }
    });
}
