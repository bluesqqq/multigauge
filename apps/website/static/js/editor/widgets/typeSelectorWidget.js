import { appendSharedSelect } from "./shared.js";

export function renderTypeSelectorWidget({ container, meta, path, targetId, setPolymorphicType }) {
	const types = Array.isArray(meta?.types?.all) ? meta.types.all : [];
	if (!types.length) return;

	const currentTypeId = typeof meta?.types?.current === "string" ? meta.types.current : null;
	const currentType = types.find((type) => type?.id === currentTypeId) ?? types[0];

	const wrap = document.createElement("label");
	wrap.className = "propertyField propertyFieldWide";

	const label = document.createElement("div");
	label.className = "propertyLabel";
	label.textContent = "Type";

	wrap.appendChild(label);
	appendSharedSelect(wrap, {
		labelText: currentType?.name || currentType?.id || "Select type",
		options: types.map((type) => ({
			name: type?.name || type?.id || "Unnamed type",
			value: type?.id || "",
			selected: type?.id === currentTypeId,
		})),
		onChange: (nextTypeId) => {
			if (typeof nextTypeId !== "string" || !nextTypeId) return;
			setPolymorphicType(targetId, path, nextTypeId);
		},
	});
	container.appendChild(wrap);
}
