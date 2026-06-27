import { appendSharedSelect, parseLayoutSizeValue, serializeLayoutSizeValue } from "./shared.js";

export function createLayoutSizeEditor({ value, onCommit, className = "", compact = false }) {
	const current = parseLayoutSizeValue(value);
	let draftValue = current.value;
	const row = document.createElement("div");
	row.className = "layoutSizeEditor" + (current.unit === "auto" ? " layoutSizeEditorAuto" : "");
	if (className) row.classList.add(className);
	if (compact) row.classList.add("layoutSizeEditorCompact");

	const unitOptions = [
		{ name: "auto", value: "auto", selected: current.unit === "auto" },
		{ name: "px", value: "px", selected: current.unit === "px" },
		{ name: "%", value: "%", selected: current.unit === "%" },
	];

	let numberInput = null;
	const getNumericValue = () => {
		if (!numberInput) return current.value;
		const nextValue = Number(numberInput.value);
		return Number.isFinite(nextValue) ? nextValue : draftValue;
	};

	const selectField = document.createElement("div");
	appendSharedSelect(selectField, {
		labelText: current.unit,
		options: unitOptions,
		onChange: (nextUnit) => {
			draftValue = getNumericValue();
			onCommit(serializeLayoutSizeValue({ unit: nextUnit, value: draftValue }));
		},
	});
	const select = selectField.firstElementChild;
	select.classList.add("layoutSizeUnitSelect");
	if (compact) select.classList.add("layoutSizeUnitSelectCompact");

	if (current.unit !== "auto") {
		numberInput = document.createElement("input");
		numberInput.type = "number";
		numberInput.value = Number.isFinite(current.value) ? String(current.value) : "0";
		numberInput.className = "layoutSizeValueInput";

		const commitNumber = () => {
			const nextValue = Number(numberInput.value);
			if (!Number.isNaN(nextValue)) {
				onCommit(serializeLayoutSizeValue({ unit: current.unit, value: nextValue }));
			}
		};

		numberInput.addEventListener("keydown", (event) => {
			if (event.key === "Enter") commitNumber();
		});
		numberInput.addEventListener("blur", commitNumber);
		numberInput.addEventListener("input", () => {
			const nextValue = Number(numberInput.value);
			if (Number.isFinite(nextValue)) {
				draftValue = nextValue;
			}
		});
	}

	if (numberInput) row.appendChild(numberInput);
	row.appendChild(select);
	return row;
}
