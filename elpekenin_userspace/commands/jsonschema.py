"""Create a jsonschema based on `elpekenin_userspace.operations`."""

from __future__ import annotations

import json
import typing as t

# for backwards compat
from typing_extensions import NotRequired, Required, is_typeddict

from elpekenin_userspace import args
from elpekenin_userspace.build import RecipeJson
from elpekenin_userspace.commands import BaseCommand
from elpekenin_userspace.operations import Args
from elpekenin_userspace.result import Ok

if t.TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

    from elpekenin_userspace.result import Result

    # NOTE: these hints are likely incomplete and/or wrong

    class AnyOf(t.TypedDict):
        """Enum of types."""

        anyOf: list[JsonType]

    class Array(t.TypedDict):
        """List."""

        type: t.Literal["array"]
        items: JsonType

    class Enum(t.TypedDict):
        """Enum of values."""

        enum: list[str]

    class Primitive(t.TypedDict):
        """Basic."""

        type: JsonPrimitiveTypeNames

    JsonPrimitiveTypeNames = t.Literal[
        "boolean",
        "integer",
        "number",
        "string",
    ]

    JsonType = t.Union[
        AnyOf,
        Array,
        Enum,
        "JsonSchema",
        Primitive,
    ]

    class JsonSchema(t.TypedDict):
        """Schema."""

        type: t.Literal["object"]
        properties: dict[str, JsonType]
        required: list[str]
        additionalProperties: bool


LOCALNS = {
    "Args": Args,
    "Literal": t.Literal,
    "NotRequired": NotRequired,
}

# mypy infers this as `dict[Any, str]`, let's be more specific
PY_TO_JS: dict[type, JsonPrimitiveTypeNames] = {
    bool: "boolean",
    float: "number",
    int: "integer",
    str: "string",
}


def is_literal_type(tp: type) -> bool:
    """Check if type is Literal[...]."""
    return t.get_origin(tp) is t.Literal


def is_not_required_type(tp: type) -> bool:
    """Check if type is NotRequired[...]."""
    return t.get_origin(tp) is NotRequired


def is_required_type(tp: type) -> bool:
    """Check if type is Required[...]."""
    return t.get_origin(tp) is Required


def python_type_to_jsonschema(py_type: type) -> JsonType:
    """Convert a Python type annotation to JSON Schema type."""
    maybe_js = PY_TO_JS.get(py_type)
    if maybe_js is not None:
        return {"type": maybe_js}

    if is_literal_type(py_type):
        values = t.get_args(py_type)
        return {"enum": list(values)}

    if is_typeddict(py_type):
        return generate_schema(py_type)

    origin = t.get_origin(py_type)
    args = t.get_args(py_type)

    if origin is t.Union:
        # `Optional[T]` is `Union[T, None]` internally
        non_none_args = [a for a in args if a is not type(None)]
        optional_args_len = 2  # T, None

        if (
            # only an argument that isn't `None`
            len(non_none_args) == 1
            # we have exactly 2 args
            and len(args) == optional_args_len
        ):
            return python_type_to_jsonschema(non_none_args[0])

        return {"anyOf": [python_type_to_jsonschema(a) for a in args]}

    if origin in {list, t.List}:  # noqa: UP006  # backwards-compat runtime type
        return {
            "type": "array",
            "items": python_type_to_jsonschema(args[0]),
        }

    msg = f"Unsupported type: {py_type}"
    raise TypeError(msg)


def generate_schema(tp: type) -> JsonSchema:
    """Generate JSON Schema from a TypedDict class."""
    hints = t.get_type_hints(
        tp,
        # without this, we get errors like `name 'NotRequired' is not defined`
        # not sure why it has to be localns, and not globalns, though...
        localns=LOCALNS,
        # required to keep `[Not]Required`
        include_extras=True,
    )

    required: list[str] = []
    properties: dict[str, JsonType] = {}

    for field, field_type in hints.items():
        # unless explicitly marked as `NotRequired`, it is required
        if not is_not_required_type(field_type):
            required.append(field)

        # unwrap `[Not]Required` types
        if is_required_type(field_type) or is_not_required_type(field_type):
            field_type = t.get_args(field_type)[0]  # noqa: PLW2901  # overwrite loop var

        properties[field] = python_type_to_jsonschema(field_type)

    return {
        "type": "object",
        "properties": properties,
        "required": required,
        "additionalProperties": False,
    }


class Jsonschema(BaseCommand):
    """Create jsonschema based on type hints."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "file",
            help="file describing build steps",
            metavar="FILE",
            type=args.File(require_existence=False),
            nargs="?",
            default="build.schema",
        )
        return super().add_args(parser)

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        schema = generate_schema(RecipeJson)

        file: Path = arguments.file
        with file.open("w") as f:
            json.dump(schema, f, indent=4)
            f.write("\n")  # trailing newline

        return Ok(None)
