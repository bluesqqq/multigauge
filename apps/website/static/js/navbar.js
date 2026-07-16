const navbar = document.querySelector(".navbar");

if (navbar) {
    const menuButton = navbar.querySelector(".navbar-menu-button");
    const menuIcon = navbar.querySelector(".navbar-menu-icon");
    const account = navbar.querySelector(".navbar-account");
    const accountToggle = navbar.querySelector("[data-account-toggle]");
    const accountMenu = navbar.querySelector("[data-account-menu]");
    const accountLinks = accountMenu ? accountMenu.querySelectorAll("a") : [];

    const menuOpenSrc = menuIcon?.dataset.menuSrc;
    const menuCloseSrc = menuIcon?.dataset.closeSrc;

    const setMenuOpen = (isOpen) => {
        navbar.classList.toggle("is-open", isOpen);
        if (menuButton) {
            menuButton.setAttribute("aria-expanded", isOpen ? "true" : "false");
        }
        if (menuIcon && menuOpenSrc && menuCloseSrc) {
            menuIcon.src = isOpen ? menuCloseSrc : menuOpenSrc;
        }
    };

    const setAccountOpen = (isOpen) => {
        if (!account || !accountToggle || !accountMenu) {
            return;
        }

        account.classList.toggle("is-open", isOpen);
        accountToggle.setAttribute("aria-expanded", isOpen ? "true" : "false");

        if (isOpen) {
            accountMenu.removeAttribute("hidden");
        } else {
            accountMenu.setAttribute("hidden", "");
        }
    };

    menuButton?.addEventListener("click", () => {
        const isOpen = !navbar.classList.contains("is-open");
        setMenuOpen(isOpen);
        setAccountOpen(false);
    });

    accountToggle?.addEventListener("click", (event) => {
        event.stopPropagation();
        const isOpen = !account?.classList.contains("is-open");
        setAccountOpen(isOpen);
        setMenuOpen(false);
    });

    accountLinks.forEach((link) => {
        link.addEventListener("click", () => {
            setAccountOpen(false);
        });
    });

    document.addEventListener("click", (event) => {
        if (account && !account.contains(event.target)) {
            setAccountOpen(false);
        }

        if (navbar && !navbar.contains(event.target)) {
            setMenuOpen(false);
        }
    });

    document.addEventListener("keydown", (event) => {
        if (event.key !== "Escape") {
            return;
        }

        setAccountOpen(false);
        setMenuOpen(false);
    });
}
